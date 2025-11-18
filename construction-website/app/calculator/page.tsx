"use client"

import { useState } from "react"
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from "@/components/ui/card"
import { Button } from "@/components/ui/button"
import { Input } from "@/components/ui/input"
import { Calculator, Download, Phone } from "lucide-react"
import { calculateEstimate, formatCurrency } from "@/lib/utils"
import Link from "next/link"

export default function CalculatorPage() {
  const [formData, setFormData] = useState({
    siteSize: "",
    floors: "1",
    buildingType: "house",
    packageType: "standard",
    location: "suburban"
  })

  const [result, setResult] = useState<{
    totalCost: number
    materialCost: number
    laborCost: number
    otherCosts: number
    timeline: number
  } | null>(null)

  const handleCalculate = () => {
    if (!formData.siteSize) return

    const estimate = calculateEstimate({
      siteSize: parseFloat(formData.siteSize),
      packageType: formData.packageType,
      floors: parseInt(formData.floors),
      location: formData.location
    })

    setResult(estimate)
  }

  return (
    <div className="py-16">
      <div className="container mx-auto px-4">
        <div className="text-center mb-12">
          <h1 className="text-4xl md:text-5xl font-bold mb-4">Construction Cost Calculator</h1>
          <p className="text-gray-600 max-w-2xl mx-auto">
            Get an instant estimate for your construction project. Our calculator provides detailed cost breakdown based on your requirements.
          </p>
        </div>

        <div className="grid lg:grid-cols-2 gap-8">
          {/* Calculator Form */}
          <Card>
            <CardHeader>
              <CardTitle className="flex items-center gap-2">
                <Calculator className="text-blue-600" />
                Project Details
              </CardTitle>
              <CardDescription>
                Enter your project specifications to get an accurate cost estimate
              </CardDescription>
            </CardHeader>
            <CardContent className="space-y-6">
              <div>
                <label className="block text-sm font-medium mb-2">
                  Site Size (sq.ft) *
                </label>
                <Input
                  type="number"
                  placeholder="e.g., 1200"
                  value={formData.siteSize}
                  onChange={(e) => setFormData({ ...formData, siteSize: e.target.value })}
                />
              </div>

              <div>
                <label className="block text-sm font-medium mb-2">
                  Number of Floors *
                </label>
                <select
                  className="w-full h-10 rounded-md border border-input bg-background px-3 py-2"
                  value={formData.floors}
                  onChange={(e) => setFormData({ ...formData, floors: e.target.value })}
                >
                  <option value="1">1 Floor (Ground)</option>
                  <option value="2">2 Floors (G+1)</option>
                  <option value="3">3 Floors (G+2)</option>
                  <option value="4">4 Floors (G+3)</option>
                </select>
              </div>

              <div>
                <label className="block text-sm font-medium mb-2">
                  Building Type *
                </label>
                <select
                  className="w-full h-10 rounded-md border border-input bg-background px-3 py-2"
                  value={formData.buildingType}
                  onChange={(e) => setFormData({ ...formData, buildingType: e.target.value })}
                >
                  <option value="house">Independent House</option>
                  <option value="villa">Villa</option>
                  <option value="rental">Rental Building</option>
                </select>
              </div>

              <div>
                <label className="block text-sm font-medium mb-2">
                  Construction Package *
                </label>
                <select
                  className="w-full h-10 rounded-md border border-input bg-background px-3 py-2"
                  value={formData.packageType}
                  onChange={(e) => setFormData({ ...formData, packageType: e.target.value })}
                >
                  <option value="basic">Basic (₹1,500/sq.ft)</option>
                  <option value="standard">Standard (₹1,800/sq.ft)</option>
                  <option value="premium">Premium (₹2,200/sq.ft)</option>
                  <option value="luxury">Luxury (₹2,800/sq.ft)</option>
                </select>
              </div>

              <div>
                <label className="block text-sm font-medium mb-2">
                  Location *
                </label>
                <select
                  className="w-full h-10 rounded-md border border-input bg-background px-3 py-2"
                  value={formData.location}
                  onChange={(e) => setFormData({ ...formData, location: e.target.value })}
                >
                  <option value="urban">Urban Area (+20%)</option>
                  <option value="suburban">Suburban Area (Standard)</option>
                  <option value="rural">Rural Area (-10%)</option>
                </select>
              </div>

              <Button onClick={handleCalculate} className="w-full" size="lg">
                <Calculator className="mr-2" size={20} />
                Calculate Cost
              </Button>
            </CardContent>
          </Card>

          {/* Results */}
          <div className="space-y-6">
            {result ? (
              <>
                <Card className="bg-blue-50 border-blue-200">
                  <CardHeader>
                    <CardTitle className="text-blue-900">Estimated Total Cost</CardTitle>
                  </CardHeader>
                  <CardContent>
                    <div className="text-4xl font-bold text-blue-600">
                      {formatCurrency(result.totalCost)}
                    </div>
                    <p className="text-sm text-gray-600 mt-2">
                      For {formData.siteSize} sq.ft | {formData.floors} Floor(s)
                    </p>
                  </CardContent>
                </Card>

                <Card>
                  <CardHeader>
                    <CardTitle>Cost Breakdown</CardTitle>
                  </CardHeader>
                  <CardContent className="space-y-4">
                    <div className="flex justify-between items-center pb-3 border-b">
                      <span className="font-medium">Material Cost (60%)</span>
                      <span className="font-bold">{formatCurrency(result.materialCost)}</span>
                    </div>
                    <div className="flex justify-between items-center pb-3 border-b">
                      <span className="font-medium">Labor Cost (30%)</span>
                      <span className="font-bold">{formatCurrency(result.laborCost)}</span>
                    </div>
                    <div className="flex justify-between items-center pb-3 border-b">
                      <span className="font-medium">Other Costs (10%)</span>
                      <span className="font-bold">{formatCurrency(result.otherCosts)}</span>
                    </div>
                    <div className="flex justify-between items-center pt-2">
                      <span className="text-lg font-semibold">Total</span>
                      <span className="text-lg font-bold text-blue-600">
                        {formatCurrency(result.totalCost)}
                      </span>
                    </div>
                  </CardContent>
                </Card>

                <Card>
                  <CardHeader>
                    <CardTitle>Project Timeline</CardTitle>
                  </CardHeader>
                  <CardContent>
                    <div className="text-2xl font-bold text-gray-900">
                      {result.timeline} Months
                    </div>
                    <p className="text-sm text-gray-600 mt-2">
                      Estimated completion time based on project scope
                    </p>
                  </CardContent>
                </Card>

                <div className="grid grid-cols-2 gap-4">
                  <Button variant="outline" className="w-full">
                    <Download className="mr-2" size={18} />
                    Download PDF
                  </Button>
                  <Button asChild className="w-full">
                    <Link href="/contact">
                      <Phone className="mr-2" size={18} />
                      Get Quote
                    </Link>
                  </Button>
                </div>

                <Card className="bg-yellow-50 border-yellow-200">
                  <CardContent className="pt-6">
                    <p className="text-sm text-yellow-800">
                      <strong>Note:</strong> This is an estimated cost. Final pricing may vary based on specific requirements, site conditions, and material choices. Contact us for a detailed quotation.
                    </p>
                  </CardContent>
                </Card>
              </>
            ) : (
              <Card className="h-full flex items-center justify-center">
                <CardContent className="text-center py-12">
                  <Calculator className="mx-auto text-gray-300 mb-4" size={64} />
                  <h3 className="text-xl font-semibold mb-2">Ready to Calculate?</h3>
                  <p className="text-gray-600">
                    Fill in the project details on the left to get your instant cost estimate
                  </p>
                </CardContent>
              </Card>
            )}
          </div>
        </div>

        {/* Additional Info */}
        <div className="mt-16">
          <h2 className="text-2xl font-bold mb-6 text-center">What&apos;s Included in the Package?</h2>
          <div className="grid md:grid-cols-4 gap-6">
            <Card>
              <CardHeader>
                <CardTitle className="text-lg">Foundation & Structure</CardTitle>
              </CardHeader>
              <CardContent className="text-sm text-gray-600">
                <ul className="list-disc list-inside space-y-1">
                  <li>Excavation</li>
                  <li>PCC & Foundation</li>
                  <li>RCC Columns & Beams</li>
                  <li>Roof Slab</li>
                </ul>
              </CardContent>
            </Card>

            <Card>
              <CardHeader>
                <CardTitle className="text-lg">Masonry & Finishes</CardTitle>
              </CardHeader>
              <CardContent className="text-sm text-gray-600">
                <ul className="list-disc list-inside space-y-1">
                  <li>Brick Masonry</li>
                  <li>Internal Plaster</li>
                  <li>External Plaster</li>
                  <li>Wall Putty & Paint</li>
                </ul>
              </CardContent>
            </Card>

            <Card>
              <CardHeader>
                <CardTitle className="text-lg">Doors & Windows</CardTitle>
              </CardHeader>
              <CardContent className="text-sm text-gray-600">
                <ul className="list-disc list-inside space-y-1">
                  <li>Main Door</li>
                  <li>Internal Doors</li>
                  <li>Windows & Grills</li>
                  <li>Hardware</li>
                </ul>
              </CardContent>
            </Card>

            <Card>
              <CardHeader>
                <CardTitle className="text-lg">Utilities</CardTitle>
              </CardHeader>
              <CardContent className="text-sm text-gray-600">
                <ul className="list-disc list-inside space-y-1">
                  <li>Electrical Wiring</li>
                  <li>Plumbing</li>
                  <li>Bathroom Fittings</li>
                  <li>Kitchen Fixtures</li>
                </ul>
              </CardContent>
            </Card>
          </div>
        </div>
      </div>
    </div>
  )
}
