import requests
import base64
import io
from PIL import Image

# ─── Step 1: Load and convert image to base64 ───
image_path = "test_image.jpg"  # change to your image path

with open(image_path, "rb") as f:
    original_bytes = f.read()

print("Original size:", len(original_bytes), "bytes")

# ─── Step 2: Compress image ───
img = Image.open(io.BytesIO(original_bytes))
img.thumbnail((800, 800))

buffer = io.BytesIO()
img.save(buffer, format="JPEG", quality=85)
compressed_bytes = buffer.getvalue()

print("Compressed size:", len(compressed_bytes), "bytes")

# ─── Step 3: Encode to base64 ───
compressed_b64 = base64.b64encode(compressed_bytes).decode("utf-8")

print("Base64 size:", len(compressed_b64), "chars")

# ─── Step 4: Send POST request ───
print("\nSending to server...")

response = requests.post(
    "http://localhost:8000/rate-outfit",
    json={"image": compressed_b64}
)

print("Status:", response.status_code)
print("Response:", response.json())