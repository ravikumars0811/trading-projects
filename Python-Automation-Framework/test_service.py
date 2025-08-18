import requests
def test_health():
    r=requests.get("http://<ec2-ip>/health")
    assert r.status_code==200
    print("Health check passed")