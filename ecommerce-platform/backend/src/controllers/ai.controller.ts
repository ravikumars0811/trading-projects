import { Response, NextFunction } from 'express';
import { AIService } from '../services/ai.service';
import { AuthRequest } from '../middleware/auth';
import { AppError } from '../middleware/errorHandler';

export const getRecommendations = async (req: AuthRequest, res: Response, next: NextFunction) => {
  try {
    const limit = parseInt(req.query.limit as string) || 10;
    const recommendations = await AIService.getRecommendations(req.user.id, limit);

    res.status(200).json({
      status: 'success',
      results: recommendations.length,
      data: recommendations,
    });
  } catch (error) {
    next(error);
  }
};

export const voiceSearch = async (req: AuthRequest, res: Response, next: NextFunction) => {
  try {
    const { query } = req.body;

    if (!query) {
      return next(new AppError('Query is required', 400));
    }

    const result = await AIService.processVoiceSearch(query);

    res.status(200).json({
      status: 'success',
      data: result,
    });
  } catch (error) {
    next(error);
  }
};

export const smartSearch = async (req: AuthRequest, res: Response, next: NextFunction) => {
  try {
    const { query } = req.query;

    if (!query) {
      return next(new AppError('Query is required', 400));
    }

    const limit = parseInt(req.query.limit as string) || 20;
    const results = await AIService.smartSearch(query as string, limit);

    res.status(200).json({
      status: 'success',
      results: results.length,
      data: results,
    });
  } catch (error) {
    next(error);
  }
};

export const getSimilarProducts = async (req: AuthRequest, res: Response, next: NextFunction) => {
  try {
    const { productId } = req.params;
    const limit = parseInt(req.query.limit as string) || 10;

    const products = await AIService.getSimilarProducts(productId, limit);

    res.status(200).json({
      status: 'success',
      results: products.length,
      data: products,
    });
  } catch (error) {
    next(error);
  }
};
