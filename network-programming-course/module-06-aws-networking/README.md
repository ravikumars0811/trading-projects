# Module 6: AWS Networking for Developers

## Overview

This module covers AWS networking concepts essential for developers building cloud-native applications with a focus on performance and reliability.

## Table of Contents

1. [VPC Fundamentals](#vpc-fundamentals)
2. [Security Groups and NACLs](#security-groups-and-nacls)
3. [Load Balancing](#load-balancing)
4. [CloudFront and CDN](#cloudfront-and-cdn)
5. [Direct Connect and VPN](#direct-connect-and-vpn)
6. [Route 53 DNS](#route-53-dns)
7. [Enhanced Networking](#enhanced-networking)
8. [Container Networking](#container-networking)
9. [Lambda Networking](#lambda-networking)

---

## VPC Fundamentals

### What is a VPC?

```
Virtual Private Cloud (VPC)
┌────────────────────────────────────────────────────┐
│ VPC: 10.0.0.0/16                                   │
│                                                    │
│  ┌──────────────────┐    ┌──────────────────┐    │
│  │ Public Subnet    │    │ Private Subnet   │    │
│  │ 10.0.1.0/24      │    │ 10.0.2.0/24      │    │
│  │                  │    │                  │    │
│  │ ┌──────────┐     │    │ ┌──────────┐     │    │
│  │ │   EC2    │     │    │ │   EC2    │     │    │
│  │ │  (Web)   │     │    │ │   (DB)   │     │    │
│  │ └──────────┘     │    │ └──────────┘     │    │
│  └────┬─────────────┘    └──────────────────┘    │
│       │                                            │
│  ┌────▼─────────────┐                             │
│  │ Internet Gateway │                             │
│  └────┬─────────────┘                             │
└───────┼────────────────────────────────────────────┘
        │
    Internet
```

### CIDR Notation

```
10.0.0.0/16
│      │ └── Subnet mask (/16 = 255.255.0.0)
│      └──── IP address
└────────── Network

/16 = 65,536 IPs (2^16)
/24 = 256 IPs (2^8)
/28 = 16 IPs (2^4)

AWS VPC CIDR blocks:
- Minimum: /28 (16 IPs)
- Maximum: /16 (65,536 IPs)
- Can have multiple CIDR blocks
```

### Subnets

```
Public Subnet:
- Has route to Internet Gateway
- Resources get public IPs
- For: Web servers, load balancers

Private Subnet:
- No direct internet access
- Uses NAT Gateway for outbound
- For: Databases, application servers

Example VPC Layout:
┌────────────────────────────────────────┐
│ VPC: 10.0.0.0/16                       │
│                                        │
│ Availability Zone A                    │
│  ├─ Public:  10.0.1.0/24               │
│  └─ Private: 10.0.2.0/24               │
│                                        │
│ Availability Zone B                    │
│  ├─ Public:  10.0.3.0/24               │
│  └─ Private: 10.0.4.0/24               │
│                                        │
│ Availability Zone C                    │
│  ├─ Public:  10.0.5.0/24               │
│  └─ Private: 10.0.6.0/24               │
└────────────────────────────────────────┘

Best Practice:
- Use multiple AZs for high availability
- Separate public/private subnets
- Leave room for growth
```

### Route Tables

```
Public Subnet Route Table:
Destination      | Target
-----------------|------------------
10.0.0.0/16      | local (VPC)
0.0.0.0/0        | igw-xxxxx (Internet Gateway)

Private Subnet Route Table:
Destination      | Target
-----------------|------------------
10.0.0.0/16      | local
0.0.0.0/0        | nat-xxxxx (NAT Gateway)
```

### VPC Endpoints

```
Interface Endpoint (PrivateLink):
┌──────────┐         ┌──────────────┐
│   EC2    │────────→│ VPC Endpoint │
│ (Private)│  ENI    │   (S3, DDB)  │
└──────────┘         └──────────────┘

Gateway Endpoint:
VPC Route Table updated automatically
No additional charges

Available Services:
- Interface Endpoints: Most AWS services
- Gateway Endpoints: S3, DynamoDB

Benefits:
- No internet gateway needed
- Lower latency
- Better security
- No data transfer charges (Gateway)
```

---

## Security Groups and NACLs

### Security Groups (Stateful)

```
┌─────────────────────────────────────┐
│ Security Group: web-server-sg       │
├─────────────────────────────────────┤
│ Inbound Rules:                      │
│  Type    | Protocol | Port | Source │
│  HTTP    | TCP      | 80   | 0.0.0.0/0 │
│  HTTPS   | TCP      | 443  | 0.0.0.0/0 │
│  SSH     | TCP      | 22   | 10.0.0.0/16 │
├─────────────────────────────────────┤
│ Outbound Rules:                     │
│  Type    | Protocol | Port | Dest   │
│  All     | All      | All  | 0.0.0.0/0 │
└─────────────────────────────────────┘

Characteristics:
- Stateful (return traffic allowed)
- Allow rules only (no deny)
- Instance-level firewall
- Evaluates all rules
```

### NACLs (Network ACLs - Stateless)

```
┌─────────────────────────────────────┐
│ NACL: public-subnet-nacl            │
├─────────────────────────────────────┤
│ Inbound Rules:                      │
│  # | Type  | Protocol | Port | Source | Allow/Deny │
│  100 | HTTP  | TCP    | 80   | 0.0.0.0/0 | ALLOW │
│  110 | HTTPS | TCP    | 443  | 0.0.0.0/0 | ALLOW │
│  120 | SSH   | TCP    | 22   | 10.0.0.0/16 | ALLOW │
│  * | All   | All    | All  | 0.0.0.0/0 | DENY │
├─────────────────────────────────────┤
│ Outbound Rules:                     │
│  100 | All   | All    | All  | 0.0.0.0/0 | ALLOW │
└─────────────────────────────────────┘

Characteristics:
- Stateless (must allow return traffic)
- Allow and deny rules
- Subnet-level firewall
- Evaluates rules in order
```

### Security Group vs NACL

| Feature | Security Group | NACL |
|---------|----------------|------|
| Level | Instance | Subnet |
| State | Stateful | Stateless |
| Rules | Allow only | Allow + Deny |
| Evaluation | All rules | Ordered |
| Default | Deny inbound | Allow all |

---

## Load Balancing

### Application Load Balancer (ALB)

```
Internet
    │
    ▼
┌─────────────────────────────┐
│ Application Load Balancer   │
│ (Layer 7 - HTTP/HTTPS)      │
└────┬────────────────┬───────┘
     │                │
     ▼                ▼
┌─────────┐      ┌─────────┐
│ Target  │      │ Target  │
│ Group 1 │      │ Group 2 │
│ /api/*  │      │ /web/*  │
└─────────┘      └─────────┘

Features:
- Path-based routing
- Host-based routing
- HTTP/2, WebSocket
- SSL termination
- Sticky sessions
- Health checks
```

### Network Load Balancer (NLB)

```
Internet
    │
    ▼
┌─────────────────────────────┐
│ Network Load Balancer       │
│ (Layer 4 - TCP/UDP/TLS)     │
│ Ultra-low latency (~100μs)  │
└────┬────────────────────────┘
     │
     ▼
┌─────────────────────────────┐
│ Target Group                │
│ - EC2 instances             │
│ - IP addresses              │
│ - Lambda functions          │
└─────────────────────────────┘

Features:
- Millions of requests/sec
- Static IP addresses
- Preserve source IP
- TLS termination
- Ultra-low latency
- No security group needed
```

### Load Balancer Comparison

| Feature | ALB | NLB | CLB |
|---------|-----|-----|-----|
| Layer | 7 (HTTP) | 4 (TCP) | 4&7 |
| Latency | ~ms | ~100μs | ~ms |
| Throughput | High | Very High | Medium |
| Static IP | No | Yes | No |
| WebSocket | Yes | Yes | No |
| TLS | Yes | Yes | Yes |
| Use Case | Web apps | Gaming, HFT | Legacy |

### Cross-Zone Load Balancing

```
Without Cross-Zone:
AZ-A (2 instances)        AZ-B (3 instances)
   50% traffic               50% traffic
   25% per instance          16.7% per instance
   (Uneven!)

With Cross-Zone:
AZ-A (2 instances)        AZ-B (3 instances)
   20% per instance          20% per instance
   (Even distribution)

Enable for:
- ALB: Enabled by default
- NLB: Disabled by default (costs $)
```

---

## CloudFront and CDN

### CloudFront Architecture

```
User Request
     │
     ▼
┌─────────────────┐
│ CloudFront Edge │  ← 450+ locations worldwide
│   Location      │
└────┬────────────┘
     │ Cache miss
     ▼
┌─────────────────┐
│ Regional Edge   │  ← Tier 2 cache
│   Cache         │
└────┬────────────┘
     │ Cache miss
     ▼
┌─────────────────┐
│ Origin          │  ← S3, ALB, custom
│ (us-east-1)     │
└─────────────────┘

Latency:
- Edge hit: 10-50ms
- Regional hit: 50-100ms
- Origin: 100-500ms
```

### CloudFront Configuration

```yaml
# CloudFront Distribution (conceptual)
Distribution:
  Origins:
    - Id: myS3Origin
      DomainName: mybucket.s3.amazonaws.com
      S3OriginConfig:
        OriginAccessIdentity: cloudfront-OAI

    - Id: myALB
      DomainName: my-alb.us-east-1.elb.amazonaws.com
      CustomOriginConfig:
        HTTPPort: 80
        HTTPSPort: 443
        OriginProtocolPolicy: https-only

  CacheBehaviors:
    - PathPattern: /api/*
      TargetOriginId: myALB
      CachePolicyId: CachingDisabled  # Dynamic content

    - PathPattern: /static/*
      TargetOriginId: myS3Origin
      CachePolicyId: CachingOptimized  # Static assets
      Compress: true
      ViewerProtocolPolicy: redirect-to-https
```

### CloudFront Performance Optimization

```
1. Cache-Control Headers:
   Cache-Control: public, max-age=31536000, immutable

2. Compression:
   - Enable Gzip/Brotli
   - Automatic for text/* types

3. HTTP/2 & HTTP/3:
   - Enabled by default
   - Multiplexing, header compression

4. Origin Shield:
   - Additional caching layer
   - Reduces origin load
   - Better cache hit ratio

5. Lambda@Edge:
   - Run code at edge
   - Header manipulation
   - A/B testing
   - Authentication
```

---

## Direct Connect and VPN

### AWS Direct Connect

```
On-Premises                    AWS
───────────                    ───

┌──────────────┐              ┌─────────┐
│ Your Network │─────────────→│  VGW    │
│              │  Dedicated   │ (Virtual│
│              │  Fiber Link  │ Gateway)│
└──────────────┘  1-100 Gbps  └────┬────┘
                                   │
                              ┌────▼────┐
                              │   VPC   │
                              └─────────┘

Benefits:
- Predictable performance
- Lower latency (vs internet)
- Reduced bandwidth costs
- Private connection

Speeds:
- 50 Mbps, 100 Mbps, 200 Mbps, 300 Mbps
- 400 Mbps, 500 Mbps, 1 Gbps
- 2 Gbps, 5 Gbps, 10 Gbps, 100 Gbps

Use Cases:
- Hybrid cloud
- Large data transfers
- Real-time applications
- Compliance requirements
```

### Site-to-Site VPN

```
On-Premises                    AWS
───────────                    ───

┌──────────────┐              ┌─────────┐
│ Customer     │─────────────→│  VGW    │
│ Gateway      │  IPsec VPN   │         │
│              │  over Internet└────┬────┘
└──────────────┘                   │
                              ┌────▼────┐
                              │   VPC   │
                              └─────────┘

Characteristics:
- Up to 1.25 Gbps per tunnel
- 2 tunnels for HA
- Encrypted (IPsec)
- Cheaper than Direct Connect
- Variable latency (internet)

Setup:
1. Create Virtual Private Gateway (VGW)
2. Attach to VPC
3. Create Customer Gateway
4. Create VPN Connection
5. Download config
6. Configure on-premises router
```

### VPN Performance Tips

```
1. Use BGP (vs static routing)
   - Automatic failover
   - Better path selection

2. Enable VPN acceleration
   - Uses AWS Global Accelerator
   - Lower latency, jitter

3. Jumbo frames
   - MTU 8500 (vs 1500)
   - Higher throughput

4. Multiple tunnels
   - ECMP (Equal Cost Multi-Path)
   - Aggregate bandwidth
```

---

## Route 53 DNS

### DNS Record Types

```
A Record:
example.com → 192.0.2.1 (IPv4)

AAAA Record:
example.com → 2001:0db8::1 (IPv6)

CNAME Record:
www.example.com → example.com

ALIAS Record (AWS-specific):
example.com → my-alb.us-east-1.elb.amazonaws.com
- Like CNAME but works at apex
- No charge for queries
- Health checks supported

MX Record:
example.com → mail.example.com (priority 10)

TXT Record:
example.com → "v=spf1 ..."
```

### Routing Policies

```
1. Simple Routing:
   └─ Single resource
      example.com → 192.0.2.1

2. Weighted Routing:
   ├─ 70% → Server A (us-east-1)
   └─ 30% → Server B (us-west-2)

3. Latency-Based Routing:
   ├─ US users → us-east-1
   ├─ EU users → eu-west-1
   └─ APAC users → ap-south-1

4. Geolocation Routing:
   ├─ US → us-east-1
   ├─ Europe → eu-west-1
   └─ Default → us-east-1

5. Geoproximity Routing:
   - Route based on location + bias
   - Can shift traffic geographically

6. Failover Routing:
   ├─ Primary (healthy) → us-east-1
   └─ Secondary (backup) → us-west-2

7. Multi-Value Answer:
   - Returns multiple IPs
   - Client-side load balancing
   - Health checks
```

### Health Checks

```
Route 53 Health Check:
┌─────────────────────────────┐
│ Health Check                │
│ - Endpoint: myapp.com:80    │
│ - Protocol: HTTP            │
│ - Path: /health             │
│ - Interval: 30 seconds      │
│ - Failure threshold: 3      │
└─────────────────────────────┘
         │
         ▼
    If unhealthy → Fail over

Types:
1. Endpoint health check
   - HTTP/HTTPS/TCP
   - String matching

2. Calculated health check
   - Combine multiple checks
   - OR, AND logic

3. CloudWatch alarm
   - Based on metrics
```

---

## Enhanced Networking

### What is Enhanced Networking?

```
Standard Networking:
Application → Kernel → Driver → Hypervisor → NIC
Latency: ~100-500 μs
Bandwidth: Limited

Enhanced Networking (SR-IOV):
Application → Kernel → Driver → NIC (direct)
Latency: ~20-50 μs (5-10x better!)
Bandwidth: Up to 100 Gbps
```

### ENA (Elastic Network Adapter)

```
Supported Instances:
- C5, C6, M5, M6, R5, R6, etc.
- Most modern instance types

Benefits:
- Up to 100 Gbps bandwidth
- Lower latency
- Higher PPS (packets per second)
- Lower jitter

Check if enabled:
$ ethtool -i eth0 | grep driver
driver: ena

Enable:
aws ec2 modify-instance-attribute \
  --instance-id i-xxxxx \
  --ena-support
```

### Placement Groups

```
1. Cluster Placement Group:
   ┌─────────────────────────┐
   │ Single AZ               │
   │ ┌────┐ ┌────┐ ┌────┐   │
   │ │ EC2│ │EC2 │ │EC2 │   │
   │ └────┘ └────┘ └────┘   │
   │ Low latency, high BW    │
   └─────────────────────────┘
   - 10 Gbps single flow
   - HPC, HFT applications

2. Spread Placement Group:
   ┌─────────────────────────┐
   │ Multiple AZs            │
   │ Rack 1  Rack 2  Rack 3  │
   │ ┌────┐ ┌────┐ ┌────┐   │
   │ │EC2 │ │EC2 │ │EC2 │   │
   │ └────┘ └────┘ └────┘   │
   └─────────────────────────┘
   - Max 7 instances per AZ
   - Critical applications
   - Separate hardware

3. Partition Placement Group:
   ┌─────────────────────────┐
   │ Partition 1│Partition 2 │
   │ ┌────┬────┐│┌────┬────┐ │
   │ │EC2 │EC2 ││ │EC2 │EC2 │ │
   │ └────┴────┘│└────┴────┘ │
   └─────────────────────────┘
   - Up to 7 partitions per AZ
   - Hadoop, Cassandra, Kafka
```

### Network Performance Optimization

```
1. Enable Enhanced Networking:
   - Use ENA-enabled instances
   - C5, M5, R5, etc.

2. Use Placement Groups:
   - Cluster for low latency
   - 10 Gbps single flow

3. Jumbo Frames (MTU 9001):
   $ sudo ip link set dev eth0 mtu 9001

4. Tune TCP:
   # Increase buffer sizes
   sysctl -w net.core.rmem_max=134217728
   sysctl -w net.core.wmem_max=134217728

5. Enable RSS (Receive Side Scaling):
   $ ethtool -L eth0 combined 8

6. Use latest kernel:
   - Better drivers
   - Performance improvements
```

---

## Container Networking

### ECS (Elastic Container Service)

```
1. Bridge Mode (default):
   Host: eth0 (10.0.1.5)
     └─ docker0 bridge
        ├─ Container 1 (172.17.0.2:8080)
        └─ Container 2 (172.17.0.3:8081)

   - Port mapping required
   - Shared network namespace

2. Host Mode:
   Container shares host network
   - No port mapping
   - Better performance
   - Port conflicts possible

3. awsvpc Mode (recommended):
   Each container gets ENI
   ┌────────────────┐
   │ Container      │
   │ ENI: 10.0.1.10 │  ← Dedicated ENI
   │ Security Group │  ← Container-level SG
   └────────────────┘

   Benefits:
   - Container-level security groups
   - VPC flow logs per container
   - Better isolation
```

### EKS (Elastic Kubernetes Service)

```
Pod Networking:
┌─────────────────────────────┐
│ Worker Node                 │
│ ┌─────────────────────────┐ │
│ │ Pod 1                   │ │
│ │ Container: 10.244.1.2   │ │
│ │ ENI: 10.0.1.5          │ │
│ └─────────────────────────┘ │
│ ┌─────────────────────────┐ │
│ │ Pod 2                   │ │
│ │ Container: 10.244.1.3   │ │
│ │ ENI: 10.0.1.6          │ │
│ └─────────────────────────┘ │
└─────────────────────────────┘

CNI Plugins:
1. AWS VPC CNI (default):
   - Uses ENIs
   - Native VPC networking
   - Limited by ENI quota

2. Calico:
   - Overlay network
   - Network policies
   - More IPs available

3. Cilium:
   - eBPF-based
   - Observability
   - Service mesh
```

---

## Lambda Networking

### Lambda in VPC

```
Default (No VPC):
┌──────────────┐
│ Lambda       │ ← Internet access
│ (AWS managed)│ ← No VPC resources
└──────────────┘

Lambda in VPC:
┌─────────────────────────────┐
│ VPC                         │
│ ┌─────────────────────────┐ │
│ │ Private Subnet          │ │
│ │ ┌──────────┐           │ │
│ │ │ Lambda   │←─ Hyper-  │ │
│ │ │          │  plane ENI│ │
│ │ └──────────┘           │ │
│ └─────────────────────────┘ │
└─────────────────────────────┘

Considerations:
- Needs NAT Gateway for internet
- ENI creation overhead (cold start)
- Hyperplane ENIs (shared, faster)
```

### Lambda Performance Tips

```
1. Use Hyperplane ENIs:
   - Enabled by default (2019+)
   - Faster cold starts
   - Shared ENIs

2. Increase Memory:
   - More CPU (proportional)
   - Faster execution
   - Lower cost per execution

3. Provisioned Concurrency:
   - Pre-warmed instances
   - No cold start
   - Consistent latency

4. VPC Endpoint:
   - Access AWS services
   - No NAT Gateway needed
   - Lower latency, cost

5. Optimize Package Size:
   - Smaller = faster cold start
   - Use layers
   - Remove unused dependencies
```

---

## AWS Networking Best Practices

### Security

1. **Principle of Least Privilege**
   - Minimal security group rules
   - Use NACLs for subnet-level control
   - Separate tiers (web, app, db)

2. **Encryption**
   - TLS for data in transit
   - VPN/Direct Connect
   - Private endpoints

3. **Network Segmentation**
   - Multiple VPCs
   - VPC peering
   - Transit Gateway

### Performance

1. **Reduce Latency**
   - CloudFront for static content
   - Route 53 latency-based routing
   - Placement groups

2. **Increase Throughput**
   - Enhanced networking
   - Jumbo frames
   - Load balancing

3. **Optimize Costs**
   - VPC endpoints (vs NAT Gateway)
   - CloudFront (reduce origin load)
   - Direct Connect (high volume)

### Monitoring

```
1. VPC Flow Logs:
   - Capture IP traffic
   - CloudWatch Logs/S3
   - Troubleshooting, security

2. CloudWatch Metrics:
   - Network packets
   - Bytes in/out
   - Load balancer metrics

3. X-Ray:
   - Trace requests
   - Identify bottlenecks
   - Service map
```

---

## Key Takeaways

1. **VPC is foundational**
   - Plan CIDR carefully
   - Multi-AZ for HA
   - Public/private subnets

2. **Security groups are stateful**
   - NACLs are stateless
   - Defense in depth

3. **Choose right load balancer**
   - ALB for HTTP/HTTPS
   - NLB for TCP/ultra-low latency

4. **CloudFront reduces latency**
   - Global edge network
   - Cache static content

5. **Enhanced networking critical for performance**
   - ENA for modern instances
   - Placement groups for HPC

6. **Container networking varies**
   - awsvpc mode for ECS
   - VPC CNI for EKS

7. **Monitor everything**
   - VPC Flow Logs
   - CloudWatch
   - Cost optimization

---

## Next Steps

Continue to [Module 7: Low-Latency Application Design](../module-07-low-latency/README.md) for the final module!

## Additional Resources

- AWS VPC Documentation: https://docs.aws.amazon.com/vpc/
- AWS Well-Architected Framework
- AWS re:Invent Networking Sessions
- "Amazon Web Services in Action" by Wittig & Wittig
