import Link from "next/link"
import { Button } from "@/components/ui/button"
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from "@/components/ui/card"
import {
  Home,
  Building2,
  Warehouse,
  Calculator,
  Phone,
  MessageSquare,
  CheckCircle,
  Award,
  Users,
  Clock,
  Star
} from "lucide-react"

export default function HomePage() {
  return (
    <div className="flex flex-col">
      {/* Hero Section */}
      <section className="relative bg-gradient-to-r from-blue-600 to-blue-800 text-white">
        <div className="container mx-auto px-4 py-24 md:py-32">
          <div className="max-w-3xl">
            <h1 className="text-4xl md:text-6xl font-bold mb-6">
              Building Your Dream Home with Excellence
            </h1>
            <p className="text-xl md:text-2xl mb-8 text-blue-100">
              Quality construction, transparent pricing, and timely delivery for Independent Homes, Villas & Rental Buildings
            </p>
            <div className="flex flex-col sm:flex-row gap-4">
              <Button size="lg" asChild className="bg-white text-blue-600 hover:bg-gray-100">
                <Link href="/calculator">
                  <Calculator className="mr-2" size={20} />
                  Calculate Cost
                </Link>
              </Button>
              <Button size="lg" variant="outline" asChild className="border-white text-white hover:bg-white hover:text-blue-600">
                <Link href="/contact">
                  <Phone className="mr-2" size={20} />
                  Get Free Consultation
                </Link>
              </Button>
            </div>
          </div>
        </div>
        <div className="absolute bottom-0 left-0 right-0">
          <svg viewBox="0 0 1440 120" fill="none" xmlns="http://www.w3.org/2000/svg">
            <path d="M0 120L60 110C120 100 240 80 360 70C480 60 600 60 720 65C840 70 960 80 1080 85C1200 90 1320 90 1380 90L1440 90V120H1380C1320 120 1200 120 1080 120C960 120 840 120 720 120C600 120 480 120 360 120C240 120 120 120 60 120H0Z" fill="white"/>
          </svg>
        </div>
      </section>

      {/* Stats Section */}
      <section className="py-12 bg-gray-50">
        <div className="container mx-auto px-4">
          <div className="grid grid-cols-2 md:grid-cols-4 gap-8 text-center">
            <div>
              <div className="text-4xl font-bold text-blue-600 mb-2">500+</div>
              <div className="text-gray-600">Projects Completed</div>
            </div>
            <div>
              <div className="text-4xl font-bold text-blue-600 mb-2">15+</div>
              <div className="text-gray-600">Years Experience</div>
            </div>
            <div>
              <div className="text-4xl font-bold text-blue-600 mb-2">98%</div>
              <div className="text-gray-600">Client Satisfaction</div>
            </div>
            <div>
              <div className="text-4xl font-bold text-blue-600 mb-2">50+</div>
              <div className="text-gray-600">Expert Team</div>
            </div>
          </div>
        </div>
      </section>

      {/* Services Section */}
      <section className="py-16">
        <div className="container mx-auto px-4">
          <div className="text-center mb-12">
            <h2 className="text-3xl md:text-4xl font-bold mb-4">Our Construction Services</h2>
            <p className="text-gray-600 max-w-2xl mx-auto">
              We offer comprehensive construction solutions tailored to your needs with transparent pricing and quality materials
            </p>
          </div>

          <div className="grid md:grid-cols-3 gap-8">
            <Card className="hover:shadow-lg transition-shadow">
              <CardHeader>
                <div className="w-12 h-12 bg-blue-100 rounded-lg flex items-center justify-center mb-4">
                  <Home className="text-blue-600" size={24} />
                </div>
                <CardTitle>House Construction</CardTitle>
                <CardDescription>Starting from ₹1,500/sq.ft</CardDescription>
              </CardHeader>
              <CardContent>
                <p className="text-gray-600 mb-4">
                  Build your dream independent home with our expert team. Complete turnkey solutions from design to handover.
                </p>
                <ul className="space-y-2 mb-4">
                  <li className="flex items-start gap-2">
                    <CheckCircle size={16} className="text-green-500 mt-1 flex-shrink-0" />
                    <span className="text-sm">Custom floor plans</span>
                  </li>
                  <li className="flex items-start gap-2">
                    <CheckCircle size={16} className="text-green-500 mt-1 flex-shrink-0" />
                    <span className="text-sm">Quality materials</span>
                  </li>
                  <li className="flex items-start gap-2">
                    <CheckCircle size={16} className="text-green-500 mt-1 flex-shrink-0" />
                    <span className="text-sm">10-year warranty</span>
                  </li>
                </ul>
                <Button asChild className="w-full">
                  <Link href="/services/house-construction">Learn More</Link>
                </Button>
              </CardContent>
            </Card>

            <Card className="hover:shadow-lg transition-shadow border-blue-500">
              <CardHeader>
                <div className="w-12 h-12 bg-blue-100 rounded-lg flex items-center justify-center mb-4">
                  <Building2 className="text-blue-600" size={24} />
                </div>
                <CardTitle>Villa Construction</CardTitle>
                <CardDescription>Starting from ₹2,200/sq.ft</CardDescription>
              </CardHeader>
              <CardContent>
                <p className="text-gray-600 mb-4">
                  Luxurious villa construction with premium finishes and modern architecture for sophisticated living.
                </p>
                <ul className="space-y-2 mb-4">
                  <li className="flex items-start gap-2">
                    <CheckCircle size={16} className="text-green-500 mt-1 flex-shrink-0" />
                    <span className="text-sm">Premium architecture</span>
                  </li>
                  <li className="flex items-start gap-2">
                    <CheckCircle size={16} className="text-green-500 mt-1 flex-shrink-0" />
                    <span className="text-sm">Luxury materials</span>
                  </li>
                  <li className="flex items-start gap-2">
                    <CheckCircle size={16} className="text-green-500 mt-1 flex-shrink-0" />
                    <span className="text-sm">Modern amenities</span>
                  </li>
                </ul>
                <Button asChild className="w-full">
                  <Link href="/services/villas">Learn More</Link>
                </Button>
              </CardContent>
            </Card>

            <Card className="hover:shadow-lg transition-shadow">
              <CardHeader>
                <div className="w-12 h-12 bg-blue-100 rounded-lg flex items-center justify-center mb-4">
                  <Warehouse className="text-blue-600" size={24} />
                </div>
                <CardTitle>Rental Buildings</CardTitle>
                <CardDescription>Starting from ₹1,800/sq.ft</CardDescription>
              </CardHeader>
              <CardContent>
                <p className="text-gray-600 mb-4">
                  Multi-unit residential buildings optimized for rental income with efficient design and durable construction.
                </p>
                <ul className="space-y-2 mb-4">
                  <li className="flex items-start gap-2">
                    <CheckCircle size={16} className="text-green-500 mt-1 flex-shrink-0" />
                    <span className="text-sm">Optimized layouts</span>
                  </li>
                  <li className="flex items-start gap-2">
                    <CheckCircle size={16} className="text-green-500 mt-1 flex-shrink-0" />
                    <span className="text-sm">Durable construction</span>
                  </li>
                  <li className="flex items-start gap-2">
                    <CheckCircle size={16} className="text-green-500 mt-1 flex-shrink-0" />
                    <span className="text-sm">High ROI focus</span>
                  </li>
                </ul>
                <Button asChild className="w-full">
                  <Link href="/services/rental-buildings">Learn More</Link>
                </Button>
              </CardContent>
            </Card>
          </div>
        </div>
      </section>

      {/* Why Choose Us */}
      <section className="py-16 bg-gray-50">
        <div className="container mx-auto px-4">
          <div className="text-center mb-12">
            <h2 className="text-3xl md:text-4xl font-bold mb-4">Why Choose BuildPro?</h2>
            <p className="text-gray-600 max-w-2xl mx-auto">
              We stand out with our commitment to quality, transparency, and customer satisfaction
            </p>
          </div>

          <div className="grid md:grid-cols-2 lg:grid-cols-4 gap-8">
            <div className="text-center">
              <div className="w-16 h-16 bg-blue-100 rounded-full flex items-center justify-center mx-auto mb-4">
                <Award className="text-blue-600" size={32} />
              </div>
              <h3 className="text-xl font-semibold mb-2">Quality Assurance</h3>
              <p className="text-gray-600">
                Premium materials from trusted brands with strict quality checks at every stage
              </p>
            </div>

            <div className="text-center">
              <div className="w-16 h-16 bg-blue-100 rounded-full flex items-center justify-center mx-auto mb-4">
                <CheckCircle className="text-blue-600" size={32} />
              </div>
              <h3 className="text-xl font-semibold mb-2">Transparent Pricing</h3>
              <p className="text-gray-600">
                No hidden costs. Complete breakup of materials and labor with detailed quotations
              </p>
            </div>

            <div className="text-center">
              <div className="w-16 h-16 bg-blue-100 rounded-full flex items-center justify-center mx-auto mb-4">
                <Clock className="text-blue-600" size={32} />
              </div>
              <h3 className="text-xl font-semibold mb-2">Timely Delivery</h3>
              <p className="text-gray-600">
                On-time project completion with regular updates and milestone tracking
              </p>
            </div>

            <div className="text-center">
              <div className="w-16 h-16 bg-blue-100 rounded-full flex items-center justify-center mx-auto mb-4">
                <Users className="text-blue-600" size={32} />
              </div>
              <h3 className="text-xl font-semibold mb-2">Expert Team</h3>
              <p className="text-gray-600">
                Experienced engineers, architects, and craftsmen dedicated to your project
              </p>
            </div>
          </div>
        </div>
      </section>

      {/* Construction Packages */}
      <section className="py-16">
        <div className="container mx-auto px-4">
          <div className="text-center mb-12">
            <h2 className="text-3xl md:text-4xl font-bold mb-4">Construction Packages</h2>
            <p className="text-gray-600 max-w-2xl mx-auto">
              Choose from our carefully curated packages to match your budget and requirements
            </p>
          </div>

          <div className="grid md:grid-cols-4 gap-6">
            <Card>
              <CardHeader>
                <CardTitle>Basic</CardTitle>
                <div className="text-3xl font-bold text-blue-600">₹1,500</div>
                <CardDescription>per sq.ft</CardDescription>
              </CardHeader>
              <CardContent>
                <ul className="space-y-2">
                  <li className="flex items-start gap-2">
                    <CheckCircle size={16} className="text-green-500 mt-1" />
                    <span className="text-sm">Standard materials</span>
                  </li>
                  <li className="flex items-start gap-2">
                    <CheckCircle size={16} className="text-green-500 mt-1" />
                    <span className="text-sm">Basic finishes</span>
                  </li>
                  <li className="flex items-start gap-2">
                    <CheckCircle size={16} className="text-green-500 mt-1" />
                    <span className="text-sm">2-year warranty</span>
                  </li>
                </ul>
                <Button asChild className="w-full mt-4" variant="outline">
                  <Link href="/packages#basic">View Details</Link>
                </Button>
              </CardContent>
            </Card>

            <Card>
              <CardHeader>
                <CardTitle>Standard</CardTitle>
                <div className="text-3xl font-bold text-blue-600">₹1,800</div>
                <CardDescription>per sq.ft</CardDescription>
              </CardHeader>
              <CardContent>
                <ul className="space-y-2">
                  <li className="flex items-start gap-2">
                    <CheckCircle size={16} className="text-green-500 mt-1" />
                    <span className="text-sm">Good quality materials</span>
                  </li>
                  <li className="flex items-start gap-2">
                    <CheckCircle size={16} className="text-green-500 mt-1" />
                    <span className="text-sm">Quality finishes</span>
                  </li>
                  <li className="flex items-start gap-2">
                    <CheckCircle size={16} className="text-green-500 mt-1" />
                    <span className="text-sm">5-year warranty</span>
                  </li>
                </ul>
                <Button asChild className="w-full mt-4" variant="outline">
                  <Link href="/packages#standard">View Details</Link>
                </Button>
              </CardContent>
            </Card>

            <Card className="border-blue-500 relative">
              <div className="absolute -top-3 right-4 bg-blue-500 text-white px-3 py-1 rounded-full text-xs font-semibold">
                POPULAR
              </div>
              <CardHeader>
                <CardTitle>Premium</CardTitle>
                <div className="text-3xl font-bold text-blue-600">₹2,200</div>
                <CardDescription>per sq.ft</CardDescription>
              </CardHeader>
              <CardContent>
                <ul className="space-y-2">
                  <li className="flex items-start gap-2">
                    <CheckCircle size={16} className="text-green-500 mt-1" />
                    <span className="text-sm">Premium materials</span>
                  </li>
                  <li className="flex items-start gap-2">
                    <CheckCircle size={16} className="text-green-500 mt-1" />
                    <span className="text-sm">Designer finishes</span>
                  </li>
                  <li className="flex items-start gap-2">
                    <CheckCircle size={16} className="text-green-500 mt-1" />
                    <span className="text-sm">10-year warranty</span>
                  </li>
                </ul>
                <Button asChild className="w-full mt-4">
                  <Link href="/packages#premium">View Details</Link>
                </Button>
              </CardContent>
            </Card>

            <Card>
              <CardHeader>
                <CardTitle>Luxury</CardTitle>
                <div className="text-3xl font-bold text-blue-600">₹2,800</div>
                <CardDescription>per sq.ft</CardDescription>
              </CardHeader>
              <CardContent>
                <ul className="space-y-2">
                  <li className="flex items-start gap-2">
                    <CheckCircle size={16} className="text-green-500 mt-1" />
                    <span className="text-sm">Luxury materials</span>
                  </li>
                  <li className="flex items-start gap-2">
                    <CheckCircle size={16} className="text-green-500 mt-1" />
                    <span className="text-sm">High-end finishes</span>
                  </li>
                  <li className="flex items-start gap-2">
                    <CheckCircle size={16} className="text-green-500 mt-1" />
                    <span className="text-sm">15-year warranty</span>
                  </li>
                </ul>
                <Button asChild className="w-full mt-4" variant="outline">
                  <Link href="/packages#luxury">View Details</Link>
                </Button>
              </CardContent>
            </Card>
          </div>
        </div>
      </section>

      {/* Testimonials Preview */}
      <section className="py-16 bg-gray-50">
        <div className="container mx-auto px-4">
          <div className="text-center mb-12">
            <h2 className="text-3xl md:text-4xl font-bold mb-4">What Our Clients Say</h2>
            <p className="text-gray-600 max-w-2xl mx-auto">
              Don&apos;t just take our word for it - hear from our satisfied customers
            </p>
          </div>

          <div className="grid md:grid-cols-3 gap-8">
            {[1, 2, 3].map((i) => (
              <Card key={i}>
                <CardContent className="pt-6">
                  <div className="flex mb-4">
                    {[1, 2, 3, 4, 5].map((star) => (
                      <Star key={star} className="text-yellow-400 fill-yellow-400" size={20} />
                    ))}
                  </div>
                  <p className="text-gray-600 mb-4">
                    &quot;Excellent work quality and on-time delivery. The team was professional and kept us updated throughout the construction process.&quot;
                  </p>
                  <div className="flex items-center gap-3">
                    <div className="w-12 h-12 bg-gray-200 rounded-full flex items-center justify-center">
                      <Users size={24} className="text-gray-400" />
                    </div>
                    <div>
                      <div className="font-semibold">Client Name</div>
                      <div className="text-sm text-gray-500">Villa Project</div>
                    </div>
                  </div>
                </CardContent>
              </Card>
            ))}
          </div>

          <div className="text-center mt-8">
            <Button asChild variant="outline">
              <Link href="/testimonials">View All Testimonials</Link>
            </Button>
          </div>
        </div>
      </section>

      {/* CTA Section */}
      <section className="py-16 bg-blue-600 text-white">
        <div className="container mx-auto px-4 text-center">
          <h2 className="text-3xl md:text-4xl font-bold mb-4">Ready to Start Your Project?</h2>
          <p className="text-xl mb-8 text-blue-100 max-w-2xl mx-auto">
            Get a free consultation and detailed cost estimate for your construction project
          </p>
          <div className="flex flex-col sm:flex-row gap-4 justify-center">
            <Button size="lg" asChild className="bg-white text-blue-600 hover:bg-gray-100">
              <Link href="/calculator">
                <Calculator className="mr-2" size={20} />
                Calculate Cost Now
              </Link>
            </Button>
            <Button size="lg" variant="outline" asChild className="border-white text-white hover:bg-white hover:text-blue-600">
              <Link href="/contact">
                <MessageSquare className="mr-2" size={20} />
                Request Callback
              </Link>
            </Button>
          </div>
        </div>
      </section>
    </div>
  )
}
