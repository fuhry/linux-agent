#!/usr/bin/env python3
import http.server
import shutil
import json

SEND_204_ON_GET = False

# Set this to location of a file containing valid Job JSON
JOB_FILE_PATH = "jobsource"

class JobTestHandler(http.server.BaseHTTPRequestHandler):

	def do_GET(self):
		if SEND_204_ON_GET:
			self.send_response(204)
			self.end_headers()
		else:
			self.send_response(200)
			self.send_header("Content-type", "application/json")
			self.end_headers()
			with open(JOB_FILE_PATH, mode='rb') as jobFile:
				shutil.copyfileobj(jobFile, self.wfile)

	def do_POST(self):
		self.send_response(204)
		self.end_headers()
		json_str = self.rfile.read().decode()
		json_obj = json.loads(json_str)
		print("Got response from " + self.client_address[0])
		print(json.dumps(json_obj, indent=4))

if __name__ == "__main__":
	server_address = ('', 8000)
	httpd = http.server.HTTPServer(server_address, JobTestHandler)
	httpd.serve_forever()
