import datetime
from influxdb import InfluxDBClient

def write(user, password, dbname, host, port, body):
    client = InfluxDBClient(host,port,user,password,dbname)
    client.write_points(body)

def get_body(record):
    #very basic for now
    tag = None

    # Set Tag if available
    record_info = record["n"].split(":")
    if len(record_info) >= 2:
        tag = record_info[len(record_info) - 2]
        record["n"] = record_info[len(record_info) - 1]
    
    # Check Time and set it to now if not given
    # TODO: maybe not pass this at all?
    if "t" not in record:
        record["t"] = datetime.datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%SZ")
    
    # Check value and set to 0 if not given
    if "v" not in record:
        record["v"] = 0

    # Create base body
    base_body = { 
        "measurement": record["n"], 
        "time": record["t"], 
        "fields": { 
            "value": record["v"]
            }
        }
    
    # Add tags option if set
    if tag is not None:
        base_body["tags"] = { "sensor": tag }
    return base_body