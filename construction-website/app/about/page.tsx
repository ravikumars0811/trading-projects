import { Card, CardContent } from "@/components/ui/card"
import { Award, Users, Clock, CheckCircle, Target, Eye } from "lucide-react"

export default function AboutPage() {
  return (
    <div className="py-16">
      <div className="container mx-auto px-4">
        {/* Hero Section */}
        <div className="text-center mb-16">
          <h1 className="text-4xl md:text-5xl font-bold mb-4">About BuildPro Construction</h1>
          <p className="text-xl text-gray-600 max-w-3xl mx-auto">
            With over 15 years of excellence in construction, we&apos;ve built our reputation on quality, transparency, and customer satisfaction.
          </p>
        </div>

        {/* Company Overview */}
        <div className="grid lg:grid-cols-2 gap-12 mb-16">
          <div>
            <h2 className="text-3xl font-bold mb-4">Who We Are</h2>
            <p className="text-gray-600 mb-4">
              BuildPro Construction is a leading construction company specializing in residential construction services. We have successfully completed over 500+ projects, delivering quality homes, villas, and rental buildings to satisfied customers.
            </p>
            <p className="text-gray-600 mb-4">
              Our commitment to excellence is reflected in every project we undertake. We use only premium quality materials from trusted brands and employ skilled professionals who are passionate about their craft.
            </p>
            <p className="text-gray-600">
              What sets us apart is our transparent pricing model, timely project delivery, and unwavering focus on customer satisfaction. We believe in building long-term relationships with our clients based on trust and quality.
            </p>
          </div>

          <div className="grid grid-cols-2 gap-6">
            <Card>
              <CardContent className="pt-6 text-center">
                <div className="text-4xl font-bold text-blue-600 mb-2">500+</div>
                <div className="text-gray-600">Projects Completed</div>
              </CardContent>
            </Card>
            <Card>
              <CardContent className="pt-6 text-center">
                <div className="text-4xl font-bold text-blue-600 mb-2">15+</div>
                <div className="text-gray-600">Years of Experience</div>
              </CardContent>
            </Card>
            <Card>
              <CardContent className="pt-6 text-center">
                <div className="text-4xl font-bold text-blue-600 mb-2">98%</div>
                <div className="text-gray-600">Client Satisfaction</div>
              </CardContent>
            </Card>
            <Card>
              <CardContent className="pt-6 text-center">
                <div className="text-4xl font-bold text-blue-600 mb-2">50+</div>
                <div className="text-gray-600">Team Members</div>
              </CardContent>
            </Card>
          </div>
        </div>

        {/* Mission & Vision */}
        <div className="grid md:grid-cols-2 gap-8 mb-16">
          <Card>
            <CardContent className="pt-6">
              <div className="flex items-center gap-3 mb-4">
                <div className="w-12 h-12 bg-blue-100 rounded-lg flex items-center justify-center">
                  <Target className="text-blue-600" size={24} />
                </div>
                <h3 className="text-2xl font-bold">Our Mission</h3>
              </div>
              <p className="text-gray-600">
                To provide exceptional construction services that exceed client expectations through innovation, quality craftsmanship, and unwavering integrity. We aim to make the dream of owning a quality home accessible and affordable for everyone.
              </p>
            </CardContent>
          </Card>

          <Card>
            <CardContent className="pt-6">
              <div className="flex items-center gap-3 mb-4">
                <div className="w-12 h-12 bg-blue-100 rounded-lg flex items-center justify-center">
                  <Eye className="text-blue-600" size={24} />
                </div>
                <h3 className="text-2xl font-bold">Our Vision</h3>
              </div>
              <p className="text-gray-600">
                To become the most trusted name in residential construction, known for our commitment to quality, transparency, and customer satisfaction. We envision building sustainable communities that stand the test of time.
              </p>
            </CardContent>
          </Card>
        </div>

        {/* Core Values */}
        <div className="mb-16">
          <h2 className="text-3xl font-bold mb-8 text-center">Our Core Values</h2>
          <div className="grid md:grid-cols-4 gap-6">
            <Card>
              <CardContent className="pt-6 text-center">
                <div className="w-16 h-16 bg-blue-100 rounded-full flex items-center justify-center mx-auto mb-4">
                  <Award className="text-blue-600" size={32} />
                </div>
                <h3 className="font-semibold mb-2">Quality</h3>
                <p className="text-sm text-gray-600">
                  Uncompromising commitment to quality in materials and workmanship
                </p>
              </CardContent>
            </Card>

            <Card>
              <CardContent className="pt-6 text-center">
                <div className="w-16 h-16 bg-blue-100 rounded-full flex items-center justify-center mx-auto mb-4">
                  <CheckCircle className="text-blue-600" size={32} />
                </div>
                <h3 className="font-semibold mb-2">Transparency</h3>
                <p className="text-sm text-gray-600">
                  Clear pricing with detailed breakup and no hidden costs
                </p>
              </CardContent>
            </Card>

            <Card>
              <CardContent className="pt-6 text-center">
                <div className="w-16 h-16 bg-blue-100 rounded-full flex items-center justify-center mx-auto mb-4">
                  <Clock className="text-blue-600" size={32} />
                </div>
                <h3 className="font-semibold mb-2">Timeliness</h3>
                <p className="text-sm text-gray-600">
                  On-time project delivery with regular progress updates
                </p>
              </CardContent>
            </Card>

            <Card>
              <CardContent className="pt-6 text-center">
                <div className="w-16 h-16 bg-blue-100 rounded-full flex items-center justify-center mx-auto mb-4">
                  <Users className="text-blue-600" size={32} />
                </div>
                <h3 className="font-semibold mb-2">Customer Focus</h3>
                <p className="text-sm text-gray-600">
                  Dedicated support and personalized attention to each client
                </p>
              </CardContent>
            </Card>
          </div>
        </div>

        {/* Why Choose Us */}
        <div className="bg-gray-50 rounded-2xl p-8 md:p-12">
          <h2 className="text-3xl font-bold mb-8 text-center">Why Choose BuildPro?</h2>
          <div className="grid md:grid-cols-2 gap-6">
            {[
              "Experienced team of engineers, architects, and craftsmen",
              "Premium quality materials from trusted brands",
              "Transparent pricing with detailed cost breakdown",
              "Timely project completion with milestone tracking",
              "Quality assurance at every construction stage",
              "Comprehensive warranty on all projects",
              "Regular communication and progress updates",
              "Compliance with all building codes and regulations",
              "Customized solutions tailored to your needs",
              "Post-construction support and maintenance"
            ].map((item, index) => (
              <div key={index} className="flex items-start gap-3">
                <CheckCircle className="text-green-500 flex-shrink-0 mt-1" size={20} />
                <span className="text-gray-700">{item}</span>
              </div>
            ))}
          </div>
        </div>
      </div>
    </div>
  )
}
