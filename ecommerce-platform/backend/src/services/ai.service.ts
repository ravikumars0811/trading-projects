import * as tf from '@tensorflow/tfjs-node';
import { Configuration, OpenAIApi } from 'openai';
import Product from '../models/Product.model';
import Order from '../models/Order.model';

const configuration = new Configuration({
  apiKey: process.env.OPENAI_API_KEY,
});
const openai = new OpenAIApi(configuration);

export class AIService {
  /**
   * Get personalized product recommendations using collaborative filtering
   */
  static async getRecommendations(userId: string, limit: number = 10): Promise<any[]> {
    try {
      // Get user's order history
      const userOrders = await Order.findAll({
        where: { userId },
        order: [['createdAt', 'DESC']],
        limit: 20,
      });

      // Extract product IDs from orders
      const purchasedProductIds = new Set<string>();
      const categoryPreferences = new Map<string, number>();

      for (const order of userOrders) {
        for (const item of order.items) {
          purchasedProductIds.add(item.productId);

          // Track category preferences
          const product = await Product.findById(item.productId);
          if (product) {
            const count = categoryPreferences.get(product.category) || 0;
            categoryPreferences.set(product.category, count + 1);
          }
        }
      }

      // Get top categories
      const topCategories = Array.from(categoryPreferences.entries())
        .sort((a, b) => b[1] - a[1])
        .slice(0, 3)
        .map(([category]) => category);

      // Find similar products based on categories, excluding already purchased
      const recommendations = await Product.find({
        category: { $in: topCategories },
        _id: { $nin: Array.from(purchasedProductIds) },
        isActive: true,
      })
        .sort({ rating: -1, soldCount: -1 })
        .limit(limit);

      // If not enough recommendations, add popular products
      if (recommendations.length < limit) {
        const additionalProducts = await Product.find({
          _id: { $nin: [...Array.from(purchasedProductIds), ...recommendations.map(p => p._id)] },
          isActive: true,
        })
          .sort({ soldCount: -1, rating: -1 })
          .limit(limit - recommendations.length);

        recommendations.push(...additionalProducts);
      }

      return recommendations;
    } catch (error) {
      console.error('AI Recommendations error:', error);
      // Fallback to popular products
      return await Product.find({ isActive: true })
        .sort({ soldCount: -1, rating: -1 })
        .limit(limit);
    }
  }

  /**
   * Process voice search query using OpenAI
   */
  static async processVoiceSearch(query: string): Promise<any> {
    try {
      // Use OpenAI to understand the intent and extract product criteria
      const response = await openai.createChatCompletion({
        model: 'gpt-3.5-turbo',
        messages: [
          {
            role: 'system',
            content: `You are a helpful shopping assistant. Extract product search criteria from user queries.
            Return a JSON object with: category, priceRange (min, max), color, brand, features.
            Only include fields that are mentioned or implied in the query.`,
          },
          {
            role: 'user',
            content: query,
          },
        ],
        temperature: 0.3,
      });

      const aiResponse = response.data.choices[0].message?.content || '{}';
      const searchCriteria = JSON.parse(aiResponse);

      // Build MongoDB query from criteria
      const filter: any = { isActive: true };

      if (searchCriteria.category) {
        filter.category = new RegExp(searchCriteria.category, 'i');
      }

      if (searchCriteria.brand) {
        filter.brand = new RegExp(searchCriteria.brand, 'i');
      }

      if (searchCriteria.priceRange) {
        filter.price = {};
        if (searchCriteria.priceRange.min) filter.price.$gte = searchCriteria.priceRange.min;
        if (searchCriteria.priceRange.max) filter.price.$lte = searchCriteria.priceRange.max;
      }

      if (searchCriteria.features && Array.isArray(searchCriteria.features)) {
        filter.features = { $in: searchCriteria.features };
      }

      // Search products
      const products = await Product.find(filter)
        .sort({ rating: -1, soldCount: -1 })
        .limit(20);

      return {
        query,
        criteria: searchCriteria,
        products,
      };
    } catch (error) {
      console.error('Voice search processing error:', error);
      // Fallback to simple text search
      return {
        query,
        products: await Product.find({
          $text: { $search: query },
          isActive: true,
        }).limit(20),
      };
    }
  }

  /**
   * Smart product search with autocomplete
   */
  static async smartSearch(query: string, limit: number = 20): Promise<any[]> {
    try {
      const products = await Product.find({
        $or: [
          { name: new RegExp(query, 'i') },
          { description: new RegExp(query, 'i') },
          { tags: new RegExp(query, 'i') },
          { brand: new RegExp(query, 'i') },
          { category: new RegExp(query, 'i') },
        ],
        isActive: true,
      })
        .sort({ soldCount: -1, rating: -1 })
        .limit(limit);

      return products;
    } catch (error) {
      console.error('Smart search error:', error);
      return [];
    }
  }

  /**
   * Get similar products based on product attributes
   */
  static async getSimilarProducts(productId: string, limit: number = 10): Promise<any[]> {
    try {
      const product = await Product.findById(productId);

      if (!product) {
        return [];
      }

      // Find products with same category and similar price range
      const priceRange = product.price * 0.3; // 30% price variance

      const similarProducts = await Product.find({
        _id: { $ne: productId },
        category: product.category,
        price: {
          $gte: product.price - priceRange,
          $lte: product.price + priceRange,
        },
        isActive: true,
      })
        .sort({ rating: -1 })
        .limit(limit);

      return similarProducts;
    } catch (error) {
      console.error('Similar products error:', error);
      return [];
    }
  }

  /**
   * Analyze product sentiment from reviews (placeholder for ML model)
   */
  static async analyzeSentiment(reviews: any[]): Promise<any> {
    // This would typically use a trained sentiment analysis model
    // For now, using basic analysis
    const sentiments = {
      positive: 0,
      neutral: 0,
      negative: 0,
    };

    for (const review of reviews) {
      if (review.rating >= 4) sentiments.positive++;
      else if (review.rating === 3) sentiments.neutral++;
      else sentiments.negative++;
    }

    const total = reviews.length || 1;

    return {
      positive: (sentiments.positive / total) * 100,
      neutral: (sentiments.neutral / total) * 100,
      negative: (sentiments.negative / total) * 100,
      totalReviews: total,
    };
  }

  /**
   * Generate product description using AI
   */
  static async generateProductDescription(productData: any): Promise<string> {
    try {
      const response = await openai.createChatCompletion({
        model: 'gpt-3.5-turbo',
        messages: [
          {
            role: 'system',
            content: 'You are a professional e-commerce copywriter. Create compelling product descriptions.',
          },
          {
            role: 'user',
            content: `Create a product description for: ${JSON.stringify(productData)}`,
          },
        ],
        temperature: 0.7,
        max_tokens: 300,
      });

      return response.data.choices[0].message?.content || '';
    } catch (error) {
      console.error('AI description generation error:', error);
      return '';
    }
  }
}
