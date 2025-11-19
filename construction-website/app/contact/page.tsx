"use client"

import { useState } from "react"
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from "@/components/ui/card"
import { Button } from "@/components/ui/button"
import { Input } from "@/components/ui/input"
import { MapPin, Phone, Mail, Clock, Send } from "lucide-react"

export default function ContactPage() {
  const [formData, setFormData] = useState({
    name: "",
    email: "",
    phone: "",
    subject: "",
    message: ""
  })

  const [status, setStatus] = useState<"idle" | "loading" | "success" | "error">("idle")

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault()
    setStatus("loading")

    try {
      const response = await fetch("/api/contact", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({
          ...formData,
          enquiryType: "GENERAL"
        })
      })

      if (response.ok) {
        setStatus("success")
        setFormData({ name: "", email: "", phone: "", subject: "", message: "" })
      } else {
        setStatus("error")
      }
    } catch (error) {
      setStatus("error")
    }
  }

  return (
    <div className="py-16">
      <div className="container mx-auto px-4">
        <div className="text-center mb-12">
          <h1 className="text-4xl md:text-5xl font-bold mb-4">Get In Touch</h1>
          <p className="text-xl text-gray-600 max-w-2xl mx-auto">
            Have a question or ready to start your construction project? We&apos;d love to hear from you.
          </p>
        </div>

        <div className="grid lg:grid-cols-3 gap-8">
          {/* Contact Information */}
          <div className="space-y-6">
            <Card>
              <CardHeader>
                <CardTitle>Contact Information</CardTitle>
                <CardDescription>Reach out to us through any of these channels</CardDescription>
              </CardHeader>
              <CardContent className="space-y-4">
                <div className="flex items-start gap-3">
                  <MapPin className="text-blue-600 flex-shrink-0 mt-1" size={20} />
                  <div>
                    <div className="font-medium">Address</div>
                    <div className="text-sm text-gray-600">
                      {process.env.NEXT_PUBLIC_COMPANY_ADDRESS}
                    </div>
                  </div>
                </div>

                <div className="flex items-start gap-3">
                  <Phone className="text-blue-600 flex-shrink-0 mt-1" size={20} />
                  <div>
                    <div className="font-medium">Phone</div>
                    <a href={`tel:${process.env.NEXT_PUBLIC_COMPANY_PHONE}`} className="text-sm text-gray-600 hover:text-blue-600">
                      {process.env.NEXT_PUBLIC_COMPANY_PHONE}
                    </a>
                  </div>
                </div>

                <div className="flex items-start gap-3">
                  <Mail className="text-blue-600 flex-shrink-0 mt-1" size={20} />
                  <div>
                    <div className="font-medium">Email</div>
                    <a href={`mailto:${process.env.NEXT_PUBLIC_COMPANY_EMAIL}`} className="text-sm text-gray-600 hover:text-blue-600">
                      {process.env.NEXT_PUBLIC_COMPANY_EMAIL}
                    </a>
                  </div>
                </div>

                <div className="flex items-start gap-3">
                  <Clock className="text-blue-600 flex-shrink-0 mt-1" size={20} />
                  <div>
                    <div className="font-medium">Working Hours</div>
                    <div className="text-sm text-gray-600">
                      Mon - Sat: 9:00 AM - 6:00 PM<br />
                      Sunday: Closed
                    </div>
                  </div>
                </div>
              </CardContent>
            </Card>

            <Card>
              <CardHeader>
                <CardTitle>Quick Actions</CardTitle>
              </CardHeader>
              <CardContent className="space-y-3">
                <Button asChild className="w-full" variant="outline">
                  <a href={`https://wa.me/${process.env.NEXT_PUBLIC_WHATSAPP_NUMBER?.replace(/[^0-9]/g, '')}`} target="_blank" rel="noopener noreferrer">
                    WhatsApp Us
                  </a>
                </Button>
                <Button asChild className="w-full" variant="outline">
                  <a href={`tel:${process.env.NEXT_PUBLIC_COMPANY_PHONE}`}>
                    Call Now
                  </a>
                </Button>
              </CardContent>
            </Card>
          </div>

          {/* Contact Form */}
          <div className="lg:col-span-2">
            <Card>
              <CardHeader>
                <CardTitle>Send us a Message</CardTitle>
                <CardDescription>
                  Fill out the form below and we&apos;ll get back to you within 24 hours
                </CardDescription>
              </CardHeader>
              <CardContent>
                <form onSubmit={handleSubmit} className="space-y-4">
                  <div className="grid md:grid-cols-2 gap-4">
                    <div>
                      <label className="block text-sm font-medium mb-2">
                        Your Name *
                      </label>
                      <Input
                        required
                        placeholder="John Doe"
                        value={formData.name}
                        onChange={(e) => setFormData({ ...formData, name: e.target.value })}
                      />
                    </div>

                    <div>
                      <label className="block text-sm font-medium mb-2">
                        Email Address *
                      </label>
                      <Input
                        required
                        type="email"
                        placeholder="john@example.com"
                        value={formData.email}
                        onChange={(e) => setFormData({ ...formData, email: e.target.value })}
                      />
                    </div>
                  </div>

                  <div className="grid md:grid-cols-2 gap-4">
                    <div>
                      <label className="block text-sm font-medium mb-2">
                        Phone Number *
                      </label>
                      <Input
                        required
                        type="tel"
                        placeholder="+1 234 567 8900"
                        value={formData.phone}
                        onChange={(e) => setFormData({ ...formData, phone: e.target.value })}
                      />
                    </div>

                    <div>
                      <label className="block text-sm font-medium mb-2">
                        Subject *
                      </label>
                      <Input
                        required
                        placeholder="Project Enquiry"
                        value={formData.subject}
                        onChange={(e) => setFormData({ ...formData, subject: e.target.value })}
                      />
                    </div>
                  </div>

                  <div>
                    <label className="block text-sm font-medium mb-2">
                      Message *
                    </label>
                    <textarea
                      required
                      className="w-full min-h-[150px] rounded-md border border-input bg-background px-3 py-2"
                      placeholder="Tell us about your project..."
                      value={formData.message}
                      onChange={(e) => setFormData({ ...formData, message: e.target.value })}
                    />
                  </div>

                  {status === "success" && (
                    <div className="bg-green-50 border border-green-200 rounded-md p-4 text-green-800">
                      Thank you for contacting us! We&apos;ll get back to you soon.
                    </div>
                  )}

                  {status === "error" && (
                    <div className="bg-red-50 border border-red-200 rounded-md p-4 text-red-800">
                      Something went wrong. Please try again or call us directly.
                    </div>
                  )}

                  <Button type="submit" size="lg" className="w-full" disabled={status === "loading"}>
                    <Send className="mr-2" size={20} />
                    {status === "loading" ? "Sending..." : "Send Message"}
                  </Button>
                </form>
              </CardContent>
            </Card>
          </div>
        </div>

        {/* Map */}
        <div className="mt-12">
          <Card>
            <CardHeader>
              <CardTitle>Find Us</CardTitle>
              <CardDescription>Visit our office for a face-to-face consultation</CardDescription>
            </CardHeader>
            <CardContent>
              <div className="aspect-video bg-gray-200 rounded-lg flex items-center justify-center">
                <p className="text-gray-500">
                  Google Maps Integration<br />
                  <span className="text-sm">(Add your Google Maps API key in .env to enable)</span>
                </p>
              </div>
            </CardContent>
          </Card>
        </div>
      </div>
    </div>
  )
}
