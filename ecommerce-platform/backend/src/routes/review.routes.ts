import express from 'express';

const router = express.Router();

// Review routes placeholder
router.get('/', (req, res) => {
  res.json({ status: 'success', message: 'Reviews' });
});

export default router;
