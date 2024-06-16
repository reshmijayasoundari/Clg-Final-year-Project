from google.cloud import storage
import pandas as pd
from sklearn.model_selection import train_test_split, cross_val_score
from sklearn.preprocessing import StandardScaler
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import accuracy_score, classification_report
import joblib

# Initialize a client
client = storage.Client()

# Define the bucket and file name
bucket_name = 'womensafty-bucket'
file_name = 'processed_data.csv'
bucket = client.bucket(bucket_name)
blob = bucket.blob(file_name)

# Download the file content as a string
data = blob.download_as_text()

# Convert the CSV string to a pandas DataFrame
from io import StringIO
df = pd.read_csv(StringIO(data))

# Define features (X) and target variable (y)
X = df[['temperature', 'gsr', 'spo2']]  # Example features
y = df['danger_label']  # Binary classification label (0 or 1)

# Split data into training and testing sets
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# Standardize features (optional but recommended for logistic regression)
scaler = StandardScaler()
X_train_scaled = scaler.fit_transform(X_train)
X_test_scaled = scaler.transform(X_test)

# Initialize logistic regression model
model = LogisticRegression()

# Train the model
model.fit(X_train_scaled, y_train)

# Predict on the test set
y_pred = model.predict(X_test_scaled)

# Evaluate the model
accuracy = accuracy_score(y_test, y_pred)
print(f'Accuracy: {accuracy:.2f}')

# Perform cross-validation to validate the model
cv_scores = cross_val_score(model, X_train_scaled, y_train, cv=5)
print(f'Cross-validation Mean Accuracy: {cv_scores.mean():.2f} (+/- {cv_scores.std() * 2:.2f})')

# Model deployment (serialization)
joblib.dump(model, 'logistic_regression_model.pkl')
