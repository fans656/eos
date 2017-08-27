import os

import flask
from flask import Flask

app = Flask(__name__)

@app.route('/', methods=['POST'])
def index():
    debug = False
    if 'debug' in flask.request.args:
        debug = True
    s = flask.request.get_data()
    with open('eos.img', 'wb') as f:
        f.write(s)
    if debug:
        os.system('start debug.bat')
    else:
        os.system('start run.bat')
    return 'ok\n'

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=6560, threaded=True, debug=True)
