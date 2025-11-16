import express from 'express';
import { protect, authorize } from '../middleware/auth';

const router = express.Router();

// Analytics routes placeholder
router.get('/', protect, authorize('admin'), (req, res) => {
  res.json({ status: 'success', message: 'Analytics data' });
});

export default router;
