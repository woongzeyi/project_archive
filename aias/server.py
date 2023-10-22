from flask import Flask, request, jsonify
import gspread
import datetime
from oauth2client.service_account import ServiceAccountCredentials

app = Flask(__name__)

# Google Sheets API credentials
SCOPE = ['https://www.googleapis.com/auth/spreadsheets']
SERVICE_ACCOUNT_FILE = 'ai-attendance-system-400111-e6954ed0c9d6.json'
SPREADSHEET_ID = '1OscUGbINpZIOHf9eYmmkz1RTyfq5bJOto2PeIbIaQO0'

# Initialize the Google Sheets API client
def get_google_sheets_client():
    credentials = ServiceAccountCredentials.from_json_keyfile_name(
        SERVICE_ACCOUNT_FILE, SCOPE)
    client = gspread.authorize(credentials)
    return client.open_by_key(SPREADSHEET_ID)

@app.route('/', methods=['POST'])
def upload_to_google_sheets():
    try:
        # Get the query parameters
        class_id = request.args.get('class_id')
        seat = request.args.get('seat')

        # Ensure both parameters are present
        if not seat or not class_id:
            return jsonify({'error': 'Both seat and class_id are required'}), 400

        # Connect to Google Sheets
        gc = get_google_sheets_client()

        # Select the first worksheet in the spreadsheet (you can change this)
        worksheet = gc.sheet1

        # Append the data to the Google Sheet
        data = [str(datetime.datetime.now()), class_id, seat]
        worksheet.append_row(data)

        return jsonify({'message': 'Data uploaded to Google Sheets successfully'}), 200

    except Exception as e:
        return jsonify({'error': str(e)}), 500

if __name__ == '__main__':
    app.run(debug=True)

