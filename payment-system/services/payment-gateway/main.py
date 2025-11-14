"""
Payment Gateway Service
Handles external payment integrations (Stripe, PayPal simulation)
"""
from fastapi import FastAPI, HTTPException, status
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
from decimal import Decimal
from typing import Optional
import uuid
import random
import sys
import os

sys.path.append(os.path.join(os.path.dirname(__file__), '..', '..'))
from shared.models import PaymentMethod, Currency, TransactionStatus
from shared.kafka_client import KafkaProducerClient, KafkaTopics
from shared.utils import setup_logging

logger = setup_logging("payment-gateway")
app = FastAPI(title="Payment Gateway Service", version="1.0.0")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

kafka_producer = KafkaProducerClient()


class PaymentRequest(BaseModel):
    amount: Decimal
    currency: Currency = Currency.USD
    payment_method: PaymentMethod
    card_number: Optional[str] = None
    card_cvv: Optional[str] = None
    card_expiry: Optional[str] = None
    bank_account: Optional[str] = None
    customer_id: str
    description: Optional[str] = None


class PaymentResponse(BaseModel):
    payment_id: str
    status: str
    transaction_id: str
    amount: Decimal
    currency: Currency
    reference_id: str
    message: str


@app.post("/api/v1/payment/process", response_model=PaymentResponse)
async def process_payment(payment: PaymentRequest):
    """
    Process external payment through payment gateway
    Simulates integration with Stripe/PayPal
    """
    logger.info(f"Processing payment for {payment.customer_id}: {payment.amount}")

    # Simulate payment processing
    payment_id = str(uuid.uuid4())
    transaction_id = str(uuid.uuid4())
    reference_id = f"PAY-{uuid.uuid4().hex[:12].upper()}"

    # Simulate success/failure (90% success rate)
    is_successful = random.random() < 0.9

    if is_successful:
        status_value = "completed"
        message = "Payment processed successfully"

        # Send event to Kafka
        kafka_producer.send_event(
            topic=KafkaTopics.PAYMENT_EVENTS,
            event_type="payment.processed",
            data={
                "payment_id": payment_id,
                "customer_id": payment.customer_id,
                "amount": float(payment.amount),
                "currency": payment.currency.value,
                "payment_method": payment.payment_method.value
            }
        )
    else:
        status_value = "failed"
        message = "Payment processing failed - insufficient funds or invalid card"

        kafka_producer.send_event(
            topic=KafkaTopics.PAYMENT_EVENTS,
            event_type="payment.failed",
            data={
                "payment_id": payment_id,
                "customer_id": payment.customer_id,
                "amount": float(payment.amount),
                "error": message
            }
        )

    return PaymentResponse(
        payment_id=payment_id,
        status=status_value,
        transaction_id=transaction_id,
        amount=payment.amount,
        currency=payment.currency,
        reference_id=reference_id,
        message=message
    )


@app.post("/api/v1/payment/refund")
async def process_refund(payment_id: str, amount: Decimal):
    """Process payment refund"""
    logger.info(f"Processing refund for payment {payment_id}: {amount}")

    refund_id = str(uuid.uuid4())

    kafka_producer.send_event(
        topic=KafkaTopics.PAYMENT_EVENTS,
        event_type="payment.refunded",
        data={
            "payment_id": payment_id,
            "refund_id": refund_id,
            "amount": float(amount)
        }
    )

    return {
        "refund_id": refund_id,
        "payment_id": payment_id,
        "status": "refunded",
        "amount": amount
    }


@app.get("/health")
async def health_check():
    return {"status": "healthy", "service": "payment-gateway"}


@app.get("/")
async def root():
    return {"service": "payment-gateway", "version": "1.0.0", "status": "running"}


if __name__ == "__main__":
    import uvicorn
    uvicorn.run("main:app", host="0.0.0.0", port=8004, reload=True)
