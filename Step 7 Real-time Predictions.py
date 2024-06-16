from google.cloud import aiplatform
import base64
import json
import joblib
from twilio.rest import Client

def predict_and_alert(event, context):
    # Load model and scaler
    storage_client = storage.Client()
    bucket = storage_client.bucket('womensafety-bucket')
    blob = bucket.blob('model.pkl')
    model = joblib.load(blob.download_as_bytes())

    blob = bucket.blob('scaler.pkl')
    scaler = joblib.load(blob.download_as_bytes())

    # Decode the Pub/Sub message
    message = base64.b64decode(event['data']).decode('utf-8')
    data = json.loads(message)

    # Prepare data for prediction
    input_data = [[data['temperature'], data['gsr'], data['spo2']]]
    input_data = scaler.transform(input_data)

    # Predict
    prediction = model.predict(input_data)

    # If prediction indicates danger, send SMS
    if prediction[0] == 1:  # assuming 1 indicates danger
  #      account_sid = 'id'
  #      auth_token = 'token'
        client = Client(account_sid, auth_token)
        message = client.messages.create(
            body='Alert! Woman in danger. Location: https://maps.google.com/?q=latitude,longitude',
            from_='+919789783082',
        to='+919894686664'
        )
        print('SMS sent')
		