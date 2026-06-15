from fastapi import FastAPI, WebSocket
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
from dotenv import load_dotenv
from openai import OpenAI
import json
import os

load_dotenv(override=True)
# print("ENV file location:", os.path.abspath(".env"))
# print("Key being used:", os.getenv("OPENAI_API_KEY")[:])
app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

# OpenAI client
client = OpenAI(
    api_key=os.getenv("OPENAI_API_KEY")
)

# Store connected frontend websockets
connected_clients = []

# Request model
class OutfitRequest(BaseModel):
    image: str  # base64 encoded image

@app.get("/")
def root():
    return {"status": "rohith server running"}

@app.post("/rate-outfit")
async def rate_outfit(request: OutfitRequest):
    try:
        print("1. Request received")
        print("Image size:", len(request.image), "chars")
        
        print("2. Sending to OpenAI...")
        response = client.chat.completions.create(
            model="gpt-5.4-nano",
            timeout=30, 
            max_completion_tokens=1024,
            messages=[
                {
                    "role": "user",
                    "content": [
                        {
                            "type": "image_url",
                            "image_url": {
                                "url": f"data:image/jpeg;base64,{request.image}"
                            }
                        },
                        {
                            "type": "text",
                            "text": """You are a fashion expert. Analyze this outfit and respond ONLY in this exact JSON format, nothing else:
{
  "rating": ,
  "whats_working": "",
  "improve": "",
  "tip": ""
}
Rules:
- Rating must be a number between 1 and 10
- Be honest, no sugar coating
- Keep all fields brief, one sentence each
- If the outfit is bad, say so directly"""
                        }
                    ]
                }
            ]
        )

        # Parse response
        response_text = response.choices[0].message.content
        clean = response_text.strip().replace("```json", "").replace("```", "")
        result = json.loads(clean)

        # Push suggestions to frontend via WebSocket
        for ws in connected_clients:
            try:
                await ws.send_json({
                    "whats_working": result["whats_working"],
                    "improve": result["improve"],
                    "tip": result["tip"]
                })
            except:
                pass

        # Return rating to ESP32
        return {"rating": result["rating"]}

    except Exception as e:
        return {"error": str(e)}


# WebSocket endpoint for frontend
@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()
    connected_clients.append(websocket)
    try:
        while True:
            await websocket.receive_text()
    except:
        connected_clients.remove(websocket)