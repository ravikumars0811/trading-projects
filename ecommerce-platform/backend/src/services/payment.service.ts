import Stripe from 'stripe';
import paypal from 'paypal-rest-sdk';
import { AppError } from '../middleware/errorHandler';

const stripe = new Stripe(process.env.STRIPE_SECRET_KEY || '', {
  apiVersion: '2024-11-20.acacia',
});

// Configure PayPal
paypal.configure({
  mode: process.env.PAYPAL_MODE || 'sandbox',
  client_id: process.env.PAYPAL_CLIENT_ID || '',
  client_secret: process.env.PAYPAL_CLIENT_SECRET || '',
});

export class PaymentService {
  // Stripe Payment Intent
  static async createStripePaymentIntent(amount: number, currency: string = 'usd', metadata: any = {}) {
    try {
      const paymentIntent = await stripe.paymentIntents.create({
        amount: Math.round(amount * 100), // Convert to cents
        currency,
        metadata,
        automatic_payment_methods: {
          enabled: true,
        },
      });

      return {
        clientSecret: paymentIntent.client_secret,
        paymentIntentId: paymentIntent.id,
      };
    } catch (error: any) {
      throw new AppError(`Stripe payment error: ${error.message}`, 400);
    }
  }

  // Confirm Stripe Payment
  static async confirmStripePayment(paymentIntentId: string) {
    try {
      const paymentIntent = await stripe.paymentIntents.retrieve(paymentIntentId);
      return paymentIntent;
    } catch (error: any) {
      throw new AppError(`Stripe confirmation error: ${error.message}`, 400);
    }
  }

  // Stripe Refund
  static async createStripeRefund(paymentIntentId: string, amount?: number) {
    try {
      const refund = await stripe.refunds.create({
        payment_intent: paymentIntentId,
        amount: amount ? Math.round(amount * 100) : undefined,
      });

      return refund;
    } catch (error: any) {
      throw new AppError(`Stripe refund error: ${error.message}`, 400);
    }
  }

  // PayPal Payment
  static async createPayPalPayment(amount: number, currency: string = 'USD', description: string): Promise<any> {
    return new Promise((resolve, reject) => {
      const create_payment_json = {
        intent: 'sale',
        payer: {
          payment_method: 'paypal',
        },
        redirect_urls: {
          return_url: `${process.env.FRONTEND_URL}/payment/success`,
          cancel_url: `${process.env.FRONTEND_URL}/payment/cancel`,
        },
        transactions: [
          {
            amount: {
              currency,
              total: amount.toFixed(2),
            },
            description,
          },
        ],
      };

      paypal.payment.create(create_payment_json, (error, payment) => {
        if (error) {
          reject(new AppError(`PayPal payment error: ${error.message}`, 400));
        } else {
          const approvalUrl = payment.links?.find((link) => link.rel === 'approval_url')?.href;
          resolve({
            paymentId: payment.id,
            approvalUrl,
          });
        }
      });
    });
  }

  // Execute PayPal Payment
  static async executePayPalPayment(paymentId: string, payerId: string): Promise<any> {
    return new Promise((resolve, reject) => {
      const execute_payment_json = {
        payer_id: payerId,
      };

      paypal.payment.execute(paymentId, execute_payment_json, (error, payment) => {
        if (error) {
          reject(new AppError(`PayPal execution error: ${error.message}`, 400));
        } else {
          resolve(payment);
        }
      });
    });
  }

  // PayPal Refund
  static async createPayPalRefund(saleId: string, amount: number, currency: string = 'USD'): Promise<any> {
    return new Promise((resolve, reject) => {
      const refund_details = {
        amount: {
          total: amount.toFixed(2),
          currency,
        },
      };

      paypal.sale.refund(saleId, refund_details, (error, refund) => {
        if (error) {
          reject(new AppError(`PayPal refund error: ${error.message}`, 400));
        } else {
          resolve(refund);
        }
      });
    });
  }
}
