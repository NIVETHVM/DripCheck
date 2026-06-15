import { useState, useEffect, useRef } from "react";

const WS_URL = "ws://localhost:8000/ws";

const STATUS = {
  IDLE: "idle",
  RECEIVING: "receiving",
  READY: "ready",
};

export default function MirrorUI() {
  const [status, setStatus] = useState(STATUS.IDLE);
  const [suggestions, setSuggestions] = useState(null);
  const [connected, setConnected] = useState(false);
  const [visible, setVisible] = useState(false);
  const wsRef = useRef(null);

  useEffect(() => {
    connect();
    return () => wsRef.current?.close();
  }, []);

  function connect() {
    const ws = new WebSocket(WS_URL);
    wsRef.current = ws;

    ws.onopen = () => {
      setConnected(true);
    };

    ws.onmessage = (e) => {
      const data = JSON.parse(e.data);
      setStatus(STATUS.RECEIVING);
      setVisible(false);
      setTimeout(() => {
        setSuggestions(data);
        setStatus(STATUS.READY);
        setVisible(true);
      }, 300);
    };

    ws.onclose = () => {
      setConnected(false);
      setTimeout(connect, 3000);
    };

    ws.onerror = () => ws.close();
  }

  return (
    <div style={styles.root}>
      {/* Connection dot */}
      <div style={styles.statusDot}>
        <span style={{ ...styles.dot, background: connected ? "#4ade80" : "#ef4444" }} />
        <span style={styles.statusText}>{connected ? "Connected" : "Reconnecting..."}</span>
      </div>

      {/* Title */}
      <div style={styles.header}>
        <div style={styles.title}>DRIPCHECK</div>
        <div style={styles.subtitle}>AI Outfit Analysis</div>
      </div>

      {/* Main card */}
      <div style={{ ...styles.card, opacity: visible ? 1 : 0, transform: visible ? "translateY(0)" : "translateY(16px)", transition: "all 0.5s ease" }}>
        {status === STATUS.IDLE && (
          <div style={styles.idleBox}>
            <div style={styles.micIcon}>🎤</div>
            <div style={styles.idleText}>Say <span style={styles.highlight}>"Rate my outfit"</span></div>
            <div style={styles.idleSubtext}>ESP32 is listening...</div>
          </div>
        )}

        {status === STATUS.RECEIVING && (
          <div style={styles.idleBox}>
            <div style={styles.spinner} />
            <div style={styles.idleText}>Analysing your outfit...</div>
          </div>
        )}

        {status === STATUS.READY && suggestions && (
          <div style={styles.resultsBox}>
            {/* What's working */}
            <div style={styles.row}>
              <div style={{ ...styles.tag, background: "#052e16", color: "#4ade80", borderColor: "#166534" }}>
                ✓ WORKING
              </div>
              <div style={styles.rowText}>{suggestions.whats_working}</div>
            </div>

            {/* Divider */}
            <div style={styles.divider} />

            {/* Improve */}
            <div style={styles.row}>
              <div style={{ ...styles.tag, background: "#1c1917", color: "#fbbf24", borderColor: "#78350f" }}>
                ↑ IMPROVE
              </div>
              <div style={styles.rowText}>{suggestions.improve}</div>
            </div>

            {/* Divider */}
            <div style={styles.divider} />

            {/* Tip */}
            <div style={styles.row}>
              <div style={{ ...styles.tag, background: "#0f172a", color: "#818cf8", borderColor: "#3730a3" }}>
                💡 TIP
              </div>
              <div style={styles.rowText}>{suggestions.tip}</div>
            </div>

            {/* Reset button */}
            <button
              style={styles.resetBtn}
              onClick={() => { setSuggestions(null); setStatus(STATUS.IDLE); setVisible(true); }}
            >
              Rate again
            </button>
          </div>
        )}
      </div>

      {/* Bottom watermark */}
      <div style={styles.footer}>POWERED BY GPT VISION</div>
    </div>
  );
}

// ─── Styles ───────────────────────────────────────────
const styles = {
  root: {
    minHeight: "100vh",
    background: "#000",
    color: "#fff",
    fontFamily: "'Courier New', monospace",
    display: "flex",
    flexDirection: "column",
    alignItems: "center",
    justifyContent: "center",
    padding: "24px",
    boxSizing: "border-box",
  },
  statusDot: {
    position: "fixed",
    top: 20,
    right: 20,
    display: "flex",
    alignItems: "center",
    gap: 8,
  },
  dot: {
    width: 8,
    height: 8,
    borderRadius: "50%",
    display: "inline-block",
  },
  statusText: {
    fontSize: 11,
    color: "#666",
    letterSpacing: "0.1em",
    textTransform: "uppercase",
  },
  header: {
    textAlign: "center",
    marginBottom: 40,
  },
  title: {
    fontSize: 36,
    fontWeight: "bold",
    letterSpacing: "0.3em",
    color: "#fff",
  },
  subtitle: {
    fontSize: 11,
    color: "#444",
    letterSpacing: "0.25em",
    textTransform: "uppercase",
    marginTop: 6,
  },
  card: {
    width: "100%",
    maxWidth: 480,
    background: "#0a0a0a",
    border: "1px solid #1f1f1f",
    borderRadius: 2,
    padding: "32px 28px",
    boxSizing: "border-box",
  },
  idleBox: {
    display: "flex",
    flexDirection: "column",
    alignItems: "center",
    gap: 12,
    padding: "16px 0",
  },
  micIcon: {
    fontSize: 40,
    marginBottom: 4,
  },
  idleText: {
    fontSize: 16,
    color: "#aaa",
    textAlign: "center",
    letterSpacing: "0.05em",
  },
  idleSubtext: {
    fontSize: 12,
    color: "#444",
    letterSpacing: "0.1em",
    textTransform: "uppercase",
  },
  highlight: {
    color: "#fff",
    fontStyle: "italic",
  },
  spinner: {
    width: 32,
    height: 32,
    border: "2px solid #222",
    borderTop: "2px solid #fff",
    borderRadius: "50%",
    animation: "spin 1s linear infinite",
  },
  resultsBox: {
    display: "flex",
    flexDirection: "column",
    gap: 0,
  },
  row: {
    display: "flex",
    alignItems: "flex-start",
    gap: 14,
    padding: "16px 0",
  },
  tag: {
    fontSize: 10,
    fontWeight: "bold",
    letterSpacing: "0.1em",
    padding: "4px 8px",
    border: "1px solid",
    borderRadius: 2,
    whiteSpace: "nowrap",
    flexShrink: 0,
    marginTop: 2,
  },
  rowText: {
    fontSize: 14,
    color: "#ccc",
    lineHeight: 1.6,
    letterSpacing: "0.02em",
  },
  divider: {
    height: 1,
    background: "#1a1a1a",
    margin: "0 -28px",
  },
  resetBtn: {
    marginTop: 24,
    width: "100%",
    padding: "12px",
    background: "transparent",
    border: "1px solid #333",
    color: "#666",
    fontFamily: "'Courier New', monospace",
    fontSize: 12,
    letterSpacing: "0.2em",
    textTransform: "uppercase",
    cursor: "pointer",
    transition: "all 0.2s",
  },
  footer: {
    marginTop: 32,
    fontSize: 10,
    color: "#222",
    letterSpacing: "0.2em",
  },
};
