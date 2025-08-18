# Python Automation Framework

## Overview
A Python-based framework to automate:
- **Deployment** of services on AWS EC2
- **Testing** of APIs using PyTest
- **Integration with Jenkins pipelines**

## Files
- `deploy.py` → launches EC2 + deploys service
- `test_service.py` → validates endpoints
- `Jenkinsfile` → optional CI/CD pipeline

## Run Instructions
```bash
# Deploy service
python deploy.py

# Run tests
pytest test_service.py