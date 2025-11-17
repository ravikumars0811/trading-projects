import express from 'express';
import { protect } from '../middleware/auth';

const router = express.Router();

// Cart routes placeholder
router.get('/', protect, (req, res) => {
  res.json({ status: 'success', message: 'Cart' });
});

export default router;
