import express from 'express';

const router = express.Router();

// Category routes placeholder
router.get('/', (req, res) => {
  res.json({ status: 'success', message: 'Categories list' });
});

export default router;
