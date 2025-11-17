import { Request, Response, NextFunction } from 'express';
import Product from '../models/Product.model';
import { AppError } from '../middleware/errorHandler';
import { cacheHelper } from '../config/redis';
import { AuthRequest } from '../middleware/auth';

const CACHE_TTL = 3600; // 1 hour

export const getAllProducts = async (req: Request, res: Response, next: NextFunction) => {
  try {
    const page = parseInt(req.query.page as string) || 1;
    const limit = parseInt(req.query.limit as string) || 20;
    const skip = (page - 1) * limit;

    const {
      category,
      brand,
      minPrice,
      maxPrice,
      rating,
      sortBy,
      search,
      isFeatured,
      isOnSale,
    } = req.query;

    // Build filter
    const filter: any = { isActive: true };

    if (category) filter.category = category;
    if (brand) filter.brand = brand;
    if (isFeatured) filter.isFeatured = isFeatured === 'true';
    if (isOnSale) filter.isOnSale = isOnSale === 'true';
    if (minPrice || maxPrice) {
      filter.price = {};
      if (minPrice) filter.price.$gte = parseFloat(minPrice as string);
      if (maxPrice) filter.price.$lte = parseFloat(maxPrice as string);
    }
    if (rating) filter.rating = { $gte: parseFloat(rating as string) };
    if (search) {
      filter.$text = { $search: search as string };
    }

    // Build sort
    let sort: any = {};
    if (sortBy === 'price_asc') sort.price = 1;
    else if (sortBy === 'price_desc') sort.price = -1;
    else if (sortBy === 'rating') sort.rating = -1;
    else if (sortBy === 'newest') sort.createdAt = -1;
    else if (sortBy === 'popular') sort.soldCount = -1;
    else sort.createdAt = -1;

    // Try to get from cache
    const cacheKey = `products:${JSON.stringify({ filter, sort, page, limit })}`;
    const cachedData = await cacheHelper.get(cacheKey);

    if (cachedData) {
      return res.status(200).json(cachedData);
    }

    // Get products
    const products = await Product.find(filter)
      .sort(sort)
      .skip(skip)
      .limit(limit)
      .select('-__v');

    const total = await Product.countDocuments(filter);

    const response = {
      status: 'success',
      results: products.length,
      total,
      page,
      totalPages: Math.ceil(total / limit),
      data: products,
    };

    // Cache the result
    await cacheHelper.set(cacheKey, response, CACHE_TTL);

    res.status(200).json(response);
  } catch (error) {
    next(error);
  }
};

export const getProduct = async (req: Request, res: Response, next: NextFunction) => {
  try {
    const { id } = req.params;

    // Try cache first
    const cacheKey = `product:${id}`;
    const cachedProduct = await cacheHelper.get(cacheKey);

    if (cachedProduct) {
      return res.status(200).json({
        status: 'success',
        data: cachedProduct,
      });
    }

    const product = await Product.findById(id);

    if (!product) {
      return next(new AppError('Product not found', 404));
    }

    // Increment view count
    await Product.findByIdAndUpdate(id, { $inc: { viewCount: 1 } });

    // Cache the product
    await cacheHelper.set(cacheKey, product, CACHE_TTL);

    res.status(200).json({
      status: 'success',
      data: product,
    });
  } catch (error) {
    next(error);
  }
};

export const createProduct = async (req: AuthRequest, res: Response, next: NextFunction) => {
  try {
    const productData = {
      ...req.body,
      sellerId: req.user.id,
    };

    const product = await Product.create(productData);

    // Invalidate products cache
    await cacheHelper.delPattern('products:*');

    res.status(201).json({
      status: 'success',
      data: product,
    });
  } catch (error) {
    next(error);
  }
};

export const updateProduct = async (req: AuthRequest, res: Response, next: NextFunction) => {
  try {
    const { id } = req.params;

    const product = await Product.findById(id);

    if (!product) {
      return next(new AppError('Product not found', 404));
    }

    // Check ownership or admin
    if (product.sellerId !== req.user.id && req.user.role !== 'admin') {
      return next(new AppError('Not authorized to update this product', 403));
    }

    const updatedProduct = await Product.findByIdAndUpdate(id, req.body, {
      new: true,
      runValidators: true,
    });

    // Invalidate cache
    await cacheHelper.del(`product:${id}`);
    await cacheHelper.delPattern('products:*');

    res.status(200).json({
      status: 'success',
      data: updatedProduct,
    });
  } catch (error) {
    next(error);
  }
};

export const deleteProduct = async (req: AuthRequest, res: Response, next: NextFunction) => {
  try {
    const { id } = req.params;

    const product = await Product.findById(id);

    if (!product) {
      return next(new AppError('Product not found', 404));
    }

    // Check ownership or admin
    if (product.sellerId !== req.user.id && req.user.role !== 'admin') {
      return next(new AppError('Not authorized to delete this product', 403));
    }

    await Product.findByIdAndDelete(id);

    // Invalidate cache
    await cacheHelper.del(`product:${id}`);
    await cacheHelper.delPattern('products:*');

    res.status(200).json({
      status: 'success',
      message: 'Product deleted successfully',
    });
  } catch (error) {
    next(error);
  }
};

export const getFeaturedProducts = async (req: Request, res: Response, next: NextFunction) => {
  try {
    const limit = parseInt(req.query.limit as string) || 10;

    const cacheKey = `featured-products:${limit}`;
    const cachedData = await cacheHelper.get(cacheKey);

    if (cachedData) {
      return res.status(200).json(cachedData);
    }

    const products = await Product.find({ isActive: true, isFeatured: true })
      .sort({ rating: -1 })
      .limit(limit)
      .select('-__v');

    const response = {
      status: 'success',
      results: products.length,
      data: products,
    };

    await cacheHelper.set(cacheKey, response, CACHE_TTL);

    res.status(200).json(response);
  } catch (error) {
    next(error);
  }
};
