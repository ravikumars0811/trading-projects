import express from 'express';
import { protect } from '../middleware/auth';

const router = express.Router();

// Order routes placeholder
router.get('/', protect, (req, res) => {
  res.json({ status: 'success', message: 'Orders list' });
});

export default router;
