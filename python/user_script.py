from condalf import senml_parser
from condalf import influx

body = []
db = ""

def parser_callback(record):
    global db
    global body

    db = senml_parser.get_db_name(record)
    body.append(influx.get_body(record))
    return

def process_data(data):
    global db
    db = ""
    global body
    body.clear()

    senml_parser.parse_cbor(data, parser_callback)
    influx.write("USERNAME", "PASSWORD", db, "IP_ADDRESS", "PORT", body)
    return
