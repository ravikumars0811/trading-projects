import express from 'express';
import {
  getRecommendations,
  voiceSearch,
  smartSearch,
  getSimilarProducts,
} from '../controllers/ai.controller';
import { protect } from '../middleware/auth';

const router = express.Router();

router.get('/recommendations', protect, getRecommendations);
router.post('/voice-search', protect, voiceSearch);
router.get('/search', protect, smartSearch);
router.get('/similar/:productId', getSimilarProducts);

export default router;
