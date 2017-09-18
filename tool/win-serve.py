import os

import flask
from flask import Flask

app = Flask(__name__)

@app.route('/', methods=['POST'])
def index():
    s = flask.request.get_data()
    with open('../bin/eos.img', 'wb') as f:
        f.write(s)
    os.system('start win-run.bat')
    return ''

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=6560, threaded=True, debug=True)
