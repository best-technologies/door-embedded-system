from flask import Flask, request, jsonify, render_template
import sqlite3
from datetime import datetime

app = Flask(__name__)

def init_db():
    conn = sqlite3.connect('door_system.db')
    c = conn.cursor()
    
    c.execute('''CREATE TABLE IF NOT EXISTS users (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT NOT NULL,
        fingerprint_id INTEGER UNIQUE,
        rfid_card TEXT UNIQUE,
        is_active BOOLEAN DEFAULT 1,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    )''')
    
    c.execute('''CREATE TABLE IF NOT EXISTS temp_passwords (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        password TEXT NOT NULL,
        user_name TEXT NOT NULL,
        expires_at TIMESTAMP NOT NULL,
        is_used BOOLEAN DEFAULT 0
    )''')
    
    c.execute('''CREATE TABLE IF NOT EXISTS attendance (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        user_name TEXT NOT NULL,
        action TEXT NOT NULL,
        timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        method TEXT NOT NULL
    )''')
    
    conn.commit()
    conn.close()

@app.route('/')
def dashboard():
    return render_template('dashboard.html')

@app.route('/api/authenticate', methods=['POST'])
def authenticate():
    data = request.json
    method = data.get('method')
    
    conn = sqlite3.connect('door_system.db')
    c = conn.cursor()
    
    user_name = None
    access_granted = False
    
    if method == 'fingerprint':
        fingerprint_id = data.get('fingerprint_id')
        c.execute('SELECT name FROM users WHERE fingerprint_id = ? AND is_active = 1', 
                 (fingerprint_id,))
        result = c.fetchone()
        if result:
            user_name = result[0]
            access_granted = True
    
    elif method == 'rfid':
        card_id = data.get('card_id')
        c.execute('SELECT name FROM users WHERE rfid_card = ? AND is_active = 1', 
                 (card_id,))
        result = c.fetchone()
        if result:
            user_name = result[0]
            access_granted = True
    
    conn.close()
    
    return jsonify({
        'access_granted': access_granted,
        'user_name': user_name,
        'timestamp': datetime.now().isoformat()
    })

@app.route('/api/attendance', methods=['POST'])
def log_attendance():
    data = request.json
    
    conn = sqlite3.connect('door_system.db')
    c = conn.cursor()
    
    c.execute('''INSERT INTO attendance (user_name, action, method) 
                VALUES (?, ?, ?)''', 
             (data['user_name'], data['action'], data.get('method', 'unknown')))
    
    conn.commit()
    conn.close()
    
    return jsonify({'status': 'logged'})

@app.route('/api/users', methods=['GET', 'POST'])
def manage_users():
    conn = sqlite3.connect('door_system.db')
    c = conn.cursor()
    
    if request.method == 'GET':
        c.execute('SELECT * FROM users WHERE is_active = 1')
        users = [dict(zip([col[0] for col in c.description], row)) 
                for row in c.fetchall()]
        conn.close()
        return jsonify(users)
    
    elif request.method == 'POST':
        data = request.json
        c.execute('''INSERT INTO users (name, fingerprint_id, rfid_card) 
                    VALUES (?, ?, ?)''',
                 (data['name'], data.get('fingerprint_id'), data.get('rfid_card')))
        conn.commit()
        conn.close()
        return jsonify({'status': 'user_added'})

if __name__ == '__main__':
    init_db()
    app.run(host='0.0.0.0', port=5000, debug=True)