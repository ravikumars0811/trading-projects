import { NextRequest, NextResponse } from "next/server"
import { prisma } from "@/lib/prisma"

export async function POST(request: NextRequest) {
  try {
    const body = await request.json()
    const { name, email, phone, subject, message, enquiryType = "GENERAL" } = body

    // Validate required fields
    if (!name || !email || !phone || !message) {
      return NextResponse.json(
        { error: "Missing required fields" },
        { status: 400 }
      )
    }

    // Save to database
    const enquiry = await prisma.enquiry.create({
      data: {
        name,
        email,
        phone,
        enquiryType,
        message: `Subject: ${subject || "No subject"}\n\n${message}`,
      },
    })

    // TODO: Send email notification
    // You can add email sending logic here using nodemailer

    return NextResponse.json(
      { message: "Enquiry submitted successfully", id: enquiry.id },
      { status: 200 }
    )
  } catch (error) {
    console.error("Contact form error:", error)
    return NextResponse.json(
      { error: "Failed to submit enquiry" },
      { status: 500 }
    )
  }
}
