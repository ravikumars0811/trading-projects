import { NextRequest, NextResponse } from "next/server"
import { prisma } from "@/lib/prisma"

export async function POST(request: NextRequest) {
  try {
    const body = await request.json()
    const {
      name,
      email,
      phone,
      enquiryType,
      message,
      siteSize,
      location,
      budget
    } = body

    // Validate required fields
    if (!name || !email || !phone || !enquiryType || !message) {
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
        message,
        siteSize: siteSize ? parseFloat(siteSize) : null,
        location: location || null,
        budget: budget ? parseFloat(budget) : null,
      },
    })

    return NextResponse.json(
      { message: "Enquiry submitted successfully", id: enquiry.id },
      { status: 200 }
    )
  } catch (error) {
    console.error("Enquiry submission error:", error)
    return NextResponse.json(
      { error: "Failed to submit enquiry" },
      { status: 500 }
    )
  }
}

export async function GET(request: NextRequest) {
  try {
    const searchParams = request.nextUrl.searchParams
    const status = searchParams.get("status")

    const enquiries = await prisma.enquiry.findMany({
      where: status ? { status: status as any } : {},
      orderBy: { createdAt: "desc" },
      take: 50,
    })

    return NextResponse.json(enquiries)
  } catch (error) {
    console.error("Fetch enquiries error:", error)
    return NextResponse.json(
      { error: "Failed to fetch enquiries" },
      { status: 500 }
    )
  }
}
