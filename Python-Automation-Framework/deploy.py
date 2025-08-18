import boto3, subprocess

ec2 = boto3.resource('ec2')
# Launch instance
instance = ec2.create_instances(ImageId='ami-xyz', MinCount=1, MaxCount=1, InstanceType='t2.micro')[0]
instance.wait_until_running()
print("Launched:", instance.id)

# Deploy Docker container
subprocess.run(["ssh","ubuntu@"+instance.public_dns_name,"docker run hello-world"])