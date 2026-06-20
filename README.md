# 🪞 DripCheck – AI Smart Mirror for Outfit Rating

> **Clap. Check. Drip.**

DripCheck is an AI-powered smart mirror that analyzes your outfit in real time and provides a fashion score along with personalized improvement suggestions.

Using an ESP32-CAM, clap detection, a FastAPI backend, OpenAI Vision, and a web dashboard, DripCheck transforms any mirror into your personal fashion advisor.

The system captures your image when you clap, sends it to an AI model for outfit analysis, displays a rating on a 7-segment display, and shows detailed recommendations on a frontend dashboard.

---

## ✨ Features

### 👏 Clap-to-Capture

* Stand in front of the mirror
* Clap once
* ESP32-CAM automatically captures your image

### 🤖 AI Outfit Analysis

* Uses OpenAI Vision to analyze:

  * Outfit coordination
  * Color matching
  * Overall appearance
  * Style consistency

### 🔢 Instant Rating Display

* AI generates a rating from **1–10**
* Rating is displayed on a **7-segment display**
* Instant feedback without opening any app

### 💡 Smart Recommendations

The frontend dashboard displays:

* ✅ What's Working
* 🔧 What Can Be Improved
* 🎯 Quick Style Tip

### ⚡ Real-Time Communication

* FastAPI backend
* WebSocket updates
* Live frontend synchronization

---

## 🏗️ System Architecture

```text
        👤 User
   in front of mirror
           │
           ▼
      👏 Clap
           │
           ▼
     ESP32-CAM
   Captures Image
           │
           ▼
     FastAPI Server
           │
           ▼
      OpenAI Vision
   Outfit Analysis
           │
 ┌─────────┴─────────┐
 ▼                   ▼
Rating          Suggestions
(1-10)           & Tips
 ▼                   ▼
7-Segment       Frontend UI
 Display
```

---

## 📸 How It Works

1. Stand in front of the mirror.
2. Clap to trigger image capture.
3. ESP32-CAM captures your outfit.
4. Image is sent to the FastAPI backend as base 64 string.
5. OpenAI Vision analyzes the outfit.
6. AI returns:

   * Rating (1–10)
   * What's Working
   * Improvement Suggestions
   * Style Tip
7. Rating is displayed on the 7-segment display.
8. Recommendations appear instantly on the frontend dashboard.

---

## 🛠️ Tech Stack

### Hardware

* ESP32-CAM
* Clap Detection Sensor / Microphone
* 7-Segment Display
* Smart Mirror Setup

### Backend

* Python
* FastAPI
* WebSockets
* OpenAI API

### Frontend

* React
* Real-time WebSocket Updates

### AI

* OpenAI Vision Models

---

## 📂 Project Structure

```text
DripCheck/
│
├── backend/
│   ├── main.py
│   └── .env
│
├── firmware/
│   └── esp32cam.ino
│
├── frontend/
│   ├── src/
│   └── public/
│
├── README.md
└── requirements.txt
```

---

## 🚀 Installation

### Clone the Repository

```bash
git clone https://github.com/yourusername/DripCheck.git
cd DripCheck
```

### Create a Virtual Environment

```bash
python -m venv venv
```

Activate the environment:

```bash
# Windows
venv\Scripts\activate

# Linux / macOS
source venv/bin/activate
```

### Install Dependencies

```bash
pip install -r requirements.txt
```

### Configure Environment Variables

Create a `.env` file:

```env
OPENAI_API_KEY=your_openai_api_key
```

### Start the Backend

```bash
uvicorn main:app --reload
```

### Upload ESP32 Firmware

Upload `esp32cam.ino` to your ESP32-CAM using Arduino IDE.

### Start the Frontend

```bash
npm install
npm run dev
```

---

## 📡 API Endpoints

### Health Check

```http
GET /
```

Response:

```json
{
  "status": "server running"
}
```

### Outfit Rating

```http
POST /rate-outfit
```

Request:

```json
{
  "image": "base64_image"
}
```

Response:

```json
{
  "rating": 8
}
```

### WebSocket

```text
/ws
```

Real-time updates:

```json
{
  "whats_working": "Good color coordination",
  "improve": "Try better fitting pants",
  "tip": "Add a watch for a cleaner look"
}
```

---

## 🎯 Example Output

### 7-Segment Display

```text
8
```

### Frontend Dashboard

**What's Working**

> Good color coordination and balanced outfit.

**Improve**

> Consider a better fitting shirt.

**Style Tip**

> Add a watch or bracelet for a more polished look.

---

## 🌟 Future Improvements

* Outfit history tracking
* Personalized style profiles
* Voice feedback
* Mobile application
* Wardrobe recommendations
* Trend-aware fashion suggestions
* Multi-user support

---

🤝 Contributing

Contributions are welcome.

Fork the repository
Create a feature branch
Commit changes
Open a pull request
📜 License

This project is licensed under the MIT License.
---

### "Your mirror should do more than reflect you—it should help you look your best."

**DripCheck – Clap. Check. Drip. 🪞✨**
