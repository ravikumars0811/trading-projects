import pandas as pd
import numpy as np
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import accuracy_score

# Load stock data
df = pd.read_csv("AAPL.csv")
df['Return'] = df['Close'].pct_change()
df['Signal'] = np.where(df['Return'] > 0, 1, -1)  # up=buy, down=sell

# Features: rolling averages
df['SMA5'] = df['Close'].rolling(5).mean()
df['SMA10'] = df['Close'].rolling(10).mean()
df = df.dropna()

X = df[['SMA5','SMA10']]
y = df['Signal']

# Train ML model
model = RandomForestClassifier(n_estimators=50)
model.fit(X, y)

# Generate signals for backtest
df['Predicted'] = model.predict(X)
df[['Date','Close','Predicted']].to_csv("signals.csv", index=False)
print("Signals saved to signals.csv")