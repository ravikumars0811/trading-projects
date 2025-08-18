from fastapi import FastAPI
import datetime

app = FastAPI()

@app.get("/health")
def health(): return {"status":"ok"}

@app.get("/time")
def time(): return {"time": str(datetime.datetime.utcnow())}