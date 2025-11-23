#!/bin/bash

# Setup script for LocalStack resources
# This script creates DynamoDB table, S3 bucket, and Lambda functions

set -e

echo "Setting up LocalStack resources..."

# AWS CLI configuration for LocalStack
export AWS_ACCESS_KEY_ID=test
export AWS_SECRET_ACCESS_KEY=test
export AWS_DEFAULT_REGION=us-east-1
ENDPOINT_URL=http://localhost:4566

# Wait for LocalStack to be ready
echo "Waiting for LocalStack to be ready..."
until aws --endpoint-url=$ENDPOINT_URL s3 ls 2>/dev/null; do
  echo "Waiting for LocalStack..."
  sleep 2
done

echo "LocalStack is ready!"

# Create S3 bucket
echo "Creating S3 bucket..."
aws --endpoint-url=$ENDPOINT_URL s3 mb s3://image-storage-bucket 2>/dev/null || echo "S3 bucket already exists"

# Enable versioning on S3 bucket
aws --endpoint-url=$ENDPOINT_URL s3api put-bucket-versioning \
  --bucket image-storage-bucket \
  --versioning-configuration Status=Enabled

# Create DynamoDB table
echo "Creating DynamoDB table..."
aws --endpoint-url=$ENDPOINT_URL dynamodb create-table \
  --table-name ImageMetadata \
  --attribute-definitions \
    AttributeName=image_id,AttributeType=S \
    AttributeName=user_id,AttributeType=S \
    AttributeName=created_at,AttributeType=S \
  --key-schema \
    AttributeName=image_id,KeyType=HASH \
    AttributeName=user_id,KeyType=RANGE \
  --global-secondary-indexes \
    "[{
      \"IndexName\": \"UserIdIndex\",
      \"KeySchema\": [
        {\"AttributeName\":\"user_id\",\"KeyType\":\"HASH\"},
        {\"AttributeName\":\"created_at\",\"KeyType\":\"RANGE\"}
      ],
      \"Projection\": {\"ProjectionType\":\"ALL\"},
      \"ProvisionedThroughput\": {\"ReadCapacityUnits\":5,\"WriteCapacityUnits\":5}
    }]" \
  --provisioned-throughput ReadCapacityUnits=5,WriteCapacityUnits=5 \
  2>/dev/null || echo "DynamoDB table already exists"

echo "Waiting for DynamoDB table to be active..."
aws --endpoint-url=$ENDPOINT_URL dynamodb wait table-exists --table-name ImageMetadata

# Package Lambda functions
echo "Packaging Lambda functions..."
cd "$(dirname "$0")/.."

for lambda_dir in lambda_functions/upload lambda_functions/list_images lambda_functions/get_image lambda_functions/delete_image; do
  function_name=$(basename $lambda_dir)
  echo "Packaging $function_name..."

  # Create deployment package
  cd $lambda_dir
  zip -r /tmp/${function_name}.zip . -x "*.pyc" "__pycache__/*"
  cd ../..

  # Add common modules
  cd lambda_functions/common
  zip -r /tmp/${function_name}.zip . -x "*.pyc" "__pycache__/*"
  cd ../..
done

# Create IAM role for Lambda (LocalStack doesn't enforce IAM)
echo "Creating IAM role..."
aws --endpoint-url=$ENDPOINT_URL iam create-role \
  --role-name lambda-execution-role \
  --assume-role-policy-document '{
    "Version": "2012-10-17",
    "Statement": [{
      "Effect": "Allow",
      "Principal": {"Service": "lambda.amazonaws.com"},
      "Action": "sts:AssumeRole"
    }]
  }' 2>/dev/null || echo "IAM role already exists"

# Create Lambda functions
echo "Creating Lambda functions..."

# Upload function
aws --endpoint-url=$ENDPOINT_URL lambda create-function \
  --function-name image-upload \
  --runtime python3.9 \
  --role arn:aws:iam::000000000000:role/lambda-execution-role \
  --handler handler.lambda_handler \
  --zip-file fileb:///tmp/upload.zip \
  --timeout 30 \
  --memory-size 512 \
  --environment Variables="{
    DYNAMODB_TABLE_NAME=ImageMetadata,
    S3_BUCKET_NAME=image-storage-bucket,
    DYNAMODB_ENDPOINT=$ENDPOINT_URL,
    S3_ENDPOINT=$ENDPOINT_URL
  }" 2>/dev/null || echo "Lambda function 'image-upload' already exists"

# List images function
aws --endpoint-url=$ENDPOINT_URL lambda create-function \
  --function-name image-list \
  --runtime python3.9 \
  --role arn:aws:iam::000000000000:role/lambda-execution-role \
  --handler handler.lambda_handler \
  --zip-file fileb:///tmp/list_images.zip \
  --timeout 30 \
  --memory-size 256 \
  --environment Variables="{
    DYNAMODB_TABLE_NAME=ImageMetadata,
    DYNAMODB_ENDPOINT=$ENDPOINT_URL
  }" 2>/dev/null || echo "Lambda function 'image-list' already exists"

# Get image function
aws --endpoint-url=$ENDPOINT_URL lambda create-function \
  --function-name image-get \
  --runtime python3.9 \
  --role arn:aws:iam::000000000000:role/lambda-execution-role \
  --handler handler.lambda_handler \
  --zip-file fileb:///tmp/get_image.zip \
  --timeout 30 \
  --memory-size 512 \
  --environment Variables="{
    DYNAMODB_TABLE_NAME=ImageMetadata,
    S3_BUCKET_NAME=image-storage-bucket,
    DYNAMODB_ENDPOINT=$ENDPOINT_URL,
    S3_ENDPOINT=$ENDPOINT_URL
  }" 2>/dev/null || echo "Lambda function 'image-get' already exists"

# Delete image function
aws --endpoint-url=$ENDPOINT_URL lambda create-function \
  --function-name image-delete \
  --runtime python3.9 \
  --role arn:aws:iam::000000000000:role/lambda-execution-role \
  --handler handler.lambda_handler \
  --zip-file fileb:///tmp/delete_image.zip \
  --timeout 30 \
  --memory-size 256 \
  --environment Variables="{
    DYNAMODB_TABLE_NAME=ImageMetadata,
    S3_BUCKET_NAME=image-storage-bucket,
    DYNAMODB_ENDPOINT=$ENDPOINT_URL,
    S3_ENDPOINT=$ENDPOINT_URL
  }" 2>/dev/null || echo "Lambda function 'image-delete' already exists"

# Create API Gateway
echo "Creating API Gateway..."
API_ID=$(aws --endpoint-url=$ENDPOINT_URL apigateway create-rest-api \
  --name image-upload-api \
  --description "Image Upload Service API" \
  --query 'id' --output text 2>/dev/null || echo "")

if [ -z "$API_ID" ]; then
  echo "API Gateway might already exist or creation failed"
else
  echo "API Gateway created with ID: $API_ID"

  # Get root resource
  ROOT_ID=$(aws --endpoint-url=$ENDPOINT_URL apigateway get-resources \
    --rest-api-id $API_ID --query 'items[0].id' --output text)

  # Create /images resource
  IMAGES_RESOURCE_ID=$(aws --endpoint-url=$ENDPOINT_URL apigateway create-resource \
    --rest-api-id $API_ID \
    --parent-id $ROOT_ID \
    --path-part images \
    --query 'id' --output text)

  # Create /images/{image_id} resource
  IMAGE_ID_RESOURCE_ID=$(aws --endpoint-url=$ENDPOINT_URL apigateway create-resource \
    --rest-api-id $API_ID \
    --parent-id $IMAGES_RESOURCE_ID \
    --path-part '{image_id}' \
    --query 'id' --output text)

  # Create methods and integrations
  # POST /images (upload)
  aws --endpoint-url=$ENDPOINT_URL apigateway put-method \
    --rest-api-id $API_ID \
    --resource-id $IMAGES_RESOURCE_ID \
    --http-method POST \
    --authorization-type NONE

  aws --endpoint-url=$ENDPOINT_URL apigateway put-integration \
    --rest-api-id $API_ID \
    --resource-id $IMAGES_RESOURCE_ID \
    --http-method POST \
    --type AWS_PROXY \
    --integration-http-method POST \
    --uri "arn:aws:apigateway:us-east-1:lambda:path/2015-03-31/functions/arn:aws:lambda:us-east-1:000000000000:function:image-upload/invocations"

  # GET /images (list)
  aws --endpoint-url=$ENDPOINT_URL apigateway put-method \
    --rest-api-id $API_ID \
    --resource-id $IMAGES_RESOURCE_ID \
    --http-method GET \
    --authorization-type NONE

  aws --endpoint-url=$ENDPOINT_URL apigateway put-integration \
    --rest-api-id $API_ID \
    --resource-id $IMAGES_RESOURCE_ID \
    --http-method GET \
    --type AWS_PROXY \
    --integration-http-method POST \
    --uri "arn:aws:apigateway:us-east-1:lambda:path/2015-03-31/functions/arn:aws:lambda:us-east-1:000000000000:function:image-list/invocations"

  # GET /images/{image_id} (get)
  aws --endpoint-url=$ENDPOINT_URL apigateway put-method \
    --rest-api-id $API_ID \
    --resource-id $IMAGE_ID_RESOURCE_ID \
    --http-method GET \
    --authorization-type NONE

  aws --endpoint-url=$ENDPOINT_URL apigateway put-integration \
    --rest-api-id $API_ID \
    --resource-id $IMAGE_ID_RESOURCE_ID \
    --http-method GET \
    --type AWS_PROXY \
    --integration-http-method POST \
    --uri "arn:aws:apigateway:us-east-1:lambda:path/2015-03-31/functions/arn:aws:lambda:us-east-1:000000000000:function:image-get/invocations"

  # DELETE /images/{image_id} (delete)
  aws --endpoint-url=$ENDPOINT_URL apigateway put-method \
    --rest-api-id $API_ID \
    --resource-id $IMAGE_ID_RESOURCE_ID \
    --http-method DELETE \
    --authorization-type NONE

  aws --endpoint-url=$ENDPOINT_URL apigateway put-integration \
    --rest-api-id $API_ID \
    --resource-id $IMAGE_ID_RESOURCE_ID \
    --http-method DELETE \
    --type AWS_PROXY \
    --integration-http-method POST \
    --uri "arn:aws:apigateway:us-east-1:lambda:path/2015-03-31/functions/arn:aws:lambda:us-east-1:000000000000:function:image-delete/invocations"

  # Deploy API
  aws --endpoint-url=$ENDPOINT_URL apigateway create-deployment \
    --rest-api-id $API_ID \
    --stage-name dev

  echo ""
  echo "========================================="
  echo "Setup completed successfully!"
  echo "========================================="
  echo "API Gateway URL: $ENDPOINT_URL/restapis/$API_ID/dev/_user_request_"
  echo ""
  echo "Available endpoints:"
  echo "  POST   $ENDPOINT_URL/restapis/$API_ID/dev/_user_request_/images"
  echo "  GET    $ENDPOINT_URL/restapis/$API_ID/dev/_user_request_/images"
  echo "  GET    $ENDPOINT_URL/restapis/$API_ID/dev/_user_request_/images/{image_id}"
  echo "  DELETE $ENDPOINT_URL/restapis/$API_ID/dev/_user_request_/images/{image_id}"
  echo "========================================="
fi

echo "LocalStack setup complete!"
