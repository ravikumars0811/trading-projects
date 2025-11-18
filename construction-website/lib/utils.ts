import { type ClassValue, clsx } from "clsx"
import { twMerge } from "tailwind-merge"

export function cn(...inputs: ClassValue[]) {
  return twMerge(clsx(inputs))
}

export function formatCurrency(amount: number): string {
  return new Intl.NumberFormat('en-IN', {
    style: 'currency',
    currency: 'INR',
    maximumFractionDigits: 0,
  }).format(amount)
}

export function formatDate(date: Date | string): string {
  return new Date(date).toLocaleDateString('en-IN', {
    year: 'numeric',
    month: 'long',
    day: 'numeric',
  })
}

export function calculateEstimate(params: {
  siteSize: number
  packageType: string
  floors: number
  location: string
}): {
  totalCost: number
  materialCost: number
  laborCost: number
  otherCosts: number
  timeline: number
} {
  const { siteSize, packageType, floors, location } = params

  // Base price per sq ft based on package
  const packagePrices: Record<string, number> = {
    basic: 1500,
    standard: 1800,
    premium: 2200,
    luxury: 2800,
  }

  // Location factor
  const locationFactors: Record<string, number> = {
    urban: 1.2,
    suburban: 1.0,
    rural: 0.9,
  }

  const basePrice = packagePrices[packageType.toLowerCase()] || 1800
  const locationFactor = locationFactors[location.toLowerCase()] || 1.0
  const floorFactor = 1 + (floors - 1) * 0.15

  const materialCost = siteSize * basePrice * locationFactor * floorFactor * 0.6
  const laborCost = siteSize * basePrice * locationFactor * floorFactor * 0.3
  const otherCosts = siteSize * basePrice * locationFactor * floorFactor * 0.1

  const totalCost = materialCost + laborCost + otherCosts
  const timeline = Math.ceil((siteSize / 1000) * 6 * floors) // months

  return {
    totalCost: Math.round(totalCost),
    materialCost: Math.round(materialCost),
    laborCost: Math.round(laborCost),
    otherCosts: Math.round(otherCosts),
    timeline,
  }
}
