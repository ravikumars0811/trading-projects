import express from 'express';
import { createPaymentIntent, confirmPayment, createRefund } from '../controllers/payment.controller';
import { protect, authorize } from '../middleware/auth';

const router = express.Router();

router.post('/create-intent', protect, createPaymentIntent);
router.post('/confirm', protect, confirmPayment);
router.post('/refund', protect, authorize('admin'), createRefund);

export default router;
