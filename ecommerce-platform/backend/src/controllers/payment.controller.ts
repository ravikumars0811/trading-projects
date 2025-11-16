import { Response, NextFunction } from 'express';
import { PaymentService } from '../services/payment.service';
import { AuthRequest } from '../middleware/auth';
import { AppError } from '../middleware/errorHandler';

export const createPaymentIntent = async (req: AuthRequest, res: Response, next: NextFunction) => {
  try {
    const { amount, currency, paymentMethod } = req.body;

    if (!amount || amount <= 0) {
      return next(new AppError('Invalid amount', 400));
    }

    let result;

    if (paymentMethod === 'stripe') {
      result = await PaymentService.createStripePaymentIntent(amount, currency, {
        userId: req.user.id,
      });
    } else if (paymentMethod === 'paypal') {
      result = await PaymentService.createPayPalPayment(amount, currency, 'Order payment');
    } else {
      return next(new AppError('Invalid payment method', 400));
    }

    res.status(200).json({
      status: 'success',
      data: result,
    });
  } catch (error) {
    next(error);
  }
};

export const confirmPayment = async (req: AuthRequest, res: Response, next: NextFunction) => {
  try {
    const { paymentIntentId, paymentMethod } = req.body;

    let result;

    if (paymentMethod === 'stripe') {
      result = await PaymentService.confirmStripePayment(paymentIntentId);
    } else if (paymentMethod === 'paypal') {
      const { payerId } = req.body;
      result = await PaymentService.executePayPalPayment(paymentIntentId, payerId);
    } else {
      return next(new AppError('Invalid payment method', 400));
    }

    res.status(200).json({
      status: 'success',
      data: result,
    });
  } catch (error) {
    next(error);
  }
};

export const createRefund = async (req: AuthRequest, res: Response, next: NextFunction) => {
  try {
    const { paymentId, amount, paymentMethod } = req.body;

    let result;

    if (paymentMethod === 'stripe') {
      result = await PaymentService.createStripeRefund(paymentId, amount);
    } else if (paymentMethod === 'paypal') {
      result = await PaymentService.createPayPalRefund(paymentId, amount);
    } else {
      return next(new AppError('Invalid payment method', 400));
    }

    res.status(200).json({
      status: 'success',
      data: result,
    });
  } catch (error) {
    next(error);
  }
};
