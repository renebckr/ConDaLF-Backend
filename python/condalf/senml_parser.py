import cbor2
import datetime

def get_db_name(record):
    record_info = record["n"].split(":")
    if len(record_info) >= 3:
        return record_info[len(record_info) - 3]
    return "main"


def parse_cbor(data, callback):
    basename = ""
    records = cbor2.loads(data)
    key_mapping = { -6: "bs", -5:"bv", -4:"bu", -3:"bt", -2:"bn", -1:"bver", 0:"n", 1:"u", 2:"v", 3:"vs", 4:"vb", 5:"s", 6:"t", 7:"ut", 8:"vd" }

    for record in records:
        translated = dict((key_mapping[key],val) for key,val in record.items())
        if "t" in translated:
            translated["t"] = datetime.datetime.utcfromtimestamp(translated["t"]).strftime("%Y-%m-%dT%H:%M:%SZ")

        # Set Basename if given
        if "bn" in translated:
            basename = translated["bn"]
            del translated["bn"]
        
        # TODO: other bases...

        # We should not evaluate records without a name (just their bases)
        if "n" not in translated:
            continue

        # If basename is given -> apply it to name
        if len(basename) != 0:
            translated["n"] = basename + translated["n"]
        
        callback(translated)
    return