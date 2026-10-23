// motion events — Node-RED function node (Motion page → "Motion log" ui-table)
// Turns motion 0/1 messages into start/end/duration rows.
// Inputs:  MQTT apartment/bedroom/esp32-01/motion {motion:0|1}
//          topic "load"  (inject once on start: redraw the table)
//          topic "clear" (ui-button: empty the log)
// Output:  full list of rows → ui-table with Action = Replace
// Needs the "file" context store in ~/.node-red/settings.js (see README).
const STORE = 'file';           // file-backed context (survives restarts)
const MAX_EVENTS = 300;         // keep the newest N events
const MERGE_GAP_MS = 60 * 1000; // motion within 60 s of the last event's end extends it
const TZ = 'America/New_York';

let events = flow.get('motionEvents', STORE) || [];   // [{startMs, endMs}], newest first
let openStart = flow.get('motionOpenStart', STORE) || null;

function fmt(ms) {
    return new Date(ms).toLocaleString('en-US', { timeZone: TZ, hour12: false });
}
function tableRows() {
    return events.map(e => ({
        Start: fmt(e.startMs),
        End: fmt(e.endMs),
        Duration: Math.round((e.endMs - e.startMs) / 1000) + ' s'
    }));
}
function save() {
    flow.set('motionEvents', events, STORE);
    flow.set('motionOpenStart', openStart, STORE);
}

if (msg.topic === 'load') {
    return { payload: tableRows() };
}
if (msg.topic === 'clear') {
    events = [];
    openStart = null;
    save();
    return { payload: [] };
}

// The broker replays the retained motion value whenever Node-RED reconnects.
// That's not a new event, so ignore it.
if (msg.retain) return null;

const motion = msg.payload && msg.payload.motion;
const now = Date.now();

if (motion === 1 && openStart === null) {
    // Reopen the last event if motion returns within the merge gap
    if (events.length && now - events[0].endMs < MERGE_GAP_MS) {
        openStart = events.shift().startMs;
    } else {
        openStart = now;
    }
    save();
    return null;                // the row is written when motion ends
}

if (motion === 0 && openStart !== null) {
    events.unshift({ startMs: openStart, endMs: now });
    events = events.slice(0, MAX_EVENTS);
    openStart = null;
    save();
    return { payload: tableRows() };
}

return null;                    // duplicate state (e.g. 0 after 0): nothing to do
