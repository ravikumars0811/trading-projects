import express from 'express';
import { protect } from '../middleware/auth';

const router = express.Router();

// User routes placeholder
router.get('/profile', protect, (req, res) => {
  res.json({ status: 'success', message: 'User profile' });
});

export default router;
