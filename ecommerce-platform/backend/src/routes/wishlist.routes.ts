import express from 'express';
import { protect } from '../middleware/auth';

const router = express.Router();

// Wishlist routes placeholder
router.get('/', protect, (req, res) => {
  res.json({ status: 'success', message: 'Wishlist' });
});

export default router;
