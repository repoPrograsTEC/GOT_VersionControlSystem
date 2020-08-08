import pymysql
from app import app
from config import mysql
from flask import jsonify
from flask import flash, request
		
@app.route('/<string:nameRepo>/lastID')
def lastID(nameRepo):
	try:
		conn = mysql.connect()
		cursor = conn.cursor(pymysql.cursors.DictCursor)
		cursor.execute("SELECT MAX(id) FROM %s_versiones" %(nameRepo))
		empRows = cursor.fetchall()
		respone = jsonify(empRows)
		respone.status_code = 200
		return respone
	except Exception as e:
		print(e)
	finally:
		cursor.close() 
		conn.close()

@app.route('/<string:nameRepo>/<string:file>/exist')
def exist(nameRepo, file):
	try:
		conn = mysql.connect()
		cursor = conn.cursor(pymysql.cursors.DictCursor)
		cursor.execute("SELECT EXISTS(SELECT 1 FROM %s_versiones WHERE archivo = '%s');" %(nameRepo, file))
		empRows = cursor.fetchall()
		respone = jsonify(empRows)
		respone.status_code = 200
		return respone
	except Exception as e:
		print(e)
	finally:
		cursor.close() 
		conn.close()

@app.route('/<string:nameRepo>/<int:id>')
def getID(nameRepo, id):
	try:
		conn = mysql.connect()
		cursor = conn.cursor(pymysql.cursors.DictCursor)
		cursor.execute("SELECT id, archivo, fecha, commit FROM %s_versiones WHERE id = %i" %(nameRepo, id))
		empRows = cursor.fetchall()
		respone = jsonify(empRows)
		respone.status_code = 200
		return respone
	except Exception as e:
		print(e)
	finally:
		cursor.close() 
		conn.close()

@app.route('/<string:nameRepo>/<string:file>/lastcommit')
def getLastCommit(nameRepo, file):
	try:
		conn = mysql.connect()
		cursor = conn.cursor(pymysql.cursors.DictCursor)
		cursor.execute("SELECT MAX(id) FROM %s_versiones WHERE archivo = '%s'" %(nameRepo, file))
		empRows = cursor.fetchall()
		respone = jsonify(empRows)
		respone.status_code = 200
		return respone
	except Exception as e:
		print(e)
	finally:
		cursor.close() 
		conn.close()

@app.route('/<string:nameRepo>/<string:file>/<string:commit>')
def getCommitFile(nameRepo, commit, file):
	try:
		conn = mysql.connect()
		cursor = conn.cursor(pymysql.cursors.DictCursor)
		cursor.execute("SELECT id, archivo, fecha, commit FROM %s_versiones WHERE commit = '%s' AND archivo = '%s'" %(nameRepo, commit, file))
		empRows = cursor.fetchall()
		respone = jsonify(empRows)
		respone.status_code = 200
		return respone
	except Exception as e:
		print(e)
	finally:
		cursor.close() 
		conn.close()

@app.route('/<string:nameRepo>/<string:commit>/status')
def getCommitStatus(nameRepo, commit):
	try:
		conn = mysql.connect()
		cursor = conn.cursor(pymysql.cursors.DictCursor)
		cursor.execute("SELECT id, archivo, fecha, commit FROM %s_versiones WHERE commit = '%s'" %(nameRepo, commit))
		empRows = cursor.fetchall()
		respone = jsonify(empRows)
		respone.status_code = 200
		return respone
	except Exception as e:
		print(e)
	finally:
		cursor.close() 
		conn.close()

@app.route('/init', methods=['POST'])
def init_repo():
	try:
		json = request.json
		name = json['name']
		sqlQuery = "CREATE TABLE `%s_versiones` (`id` int(100) NOT NULL, `archivo` varchar(255) NOT NULL,`fecha` timestamp DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,`commit` varchar(255) NOT NULL)ENGINE=InnoDB DEFAULT CHARSET=latin1"%(name)
		print(sqlQuery)
		sqlQuery2 = "CREATE TABLE `%s_datos` (`archivo` varchar(255) NOT NULL, `contenido` varchar(255) NOT NULL)"%(name)

		sqlQuery3 = "ALTER TABLE `%s_versiones` ADD PRIMARY KEY (`id`)"%(name)

		sqlQuery4 ="ALTER TABLE `%s_versiones` MODIFY `id` int(100) NOT NULL AUTO_INCREMENT"%(name)

			
		conn = mysql.connect()
		cursor = conn.cursor()
		cursor.execute(sqlQuery)
		cursor.execute(sqlQuery2)
		cursor.execute(sqlQuery3)
		cursor.execute(sqlQuery4)
		
		
		conn.commit()
		respone = jsonify('Tabla agregada correctamente')
		respone.status_code = 200
		return respone
	except Exception as e:
		print(e)

@app.route('/add', methods=['POST'])
def add():
	try:
		
		json = request.json
		nameRepo = json['nameRepo']
		name = json['name']
		commit = json['commit']

		validacion = nameRepo and name and commit and request.method == 'POST'

		if validacion:
			sqlQuery = "INSERT INTO %s_versiones (archivo, commit) VALUES('%s', '%s')" %(nameRepo, name, commit)
			print(sqlQuery)
			
			conn = mysql.connect()
			cursor = conn.cursor()
			cursor.execute(sqlQuery)
			conn.commit()
			respone = jsonify('Add correcto')
			respone.status_code = 200
			return respone
		

	except Exception as e:
		print (e)

@app.route('/post', methods=['POST'])
def add_emp():
	try:
		json = request.json
		name = json['name']
		email = json['email']
		phone = json['phone']
		address = json['address']

		validacion = name and email and phone and address and request.method == 'POST'

		if validacion:
			sqlQuery = "INSERT INTO rest_emp(name, email, phone, address) VALUES('%s', '%s', '%s', '%s')" %(name, email, phone, address)
			print(sqlQuery)
			
			conn = mysql.connect()
			cursor = conn.cursor()
			cursor.execute(sqlQuery)
			conn.commit()
			respone = jsonify('Employee added successfully!')
			respone.status_code = 200
			return respone

			"""	
			sqlQuery = "INSERT INTO rest_emp(name, email, phone, address) VALUES(%s, %s, %s, %s, %s)"
			bindData = (name, email, phone, address)
			conn = mysql.connect()
			cursor = conn.cursor()
			cursor.execute(sqlQuery, bindData)
			conn.commit()
			respone = jsonify('Employee added successfully!')
			respone.status_code = 200
			return respone
			"""
		else:
			return not_found()

	except Exception as e:
		print(e)
	finally:
		cursor.close() 
		conn.close()
		
@app.route('/getVersion/<string:nameRepo>')
def getRepo(nameRepo):
	try:
		conn = mysql.connect()
		cursor = conn.cursor(pymysql.cursors.DictCursor)
		cursor.execute("SELECT MAX(id) FROM %s_versiones" %(nameRepo))
		empRow = cursor.fetchone()
		respone = jsonify(empRow)
		respone.status_code = 200
		return respone
	except Exception as e:
		print(e)
	finally:
		cursor.close() 
		conn.close()

@app.route('/emp')
def emp():
	try:
		conn = mysql.connect()
		cursor = conn.cursor(pymysql.cursors.DictCursor)
		cursor.execute("SELECT id, name, email, phone, address FROM rest_emp")
		empRows = cursor.fetchall()
		respone = jsonify(empRows)
		respone.status_code = 200
		return respone
	except Exception as e:
		print(e)
	finally:
		cursor.close() 
		conn.close()

@app.route('/emp/<int:id>')
def emp2(id):
	try:
		conn = mysql.connect()
		cursor = conn.cursor(pymysql.cursors.DictCursor)
		cursor.execute("SELECT id, name, email, phone, address FROM rest_emp WHERE id =%s", id)
		empRow = cursor.fetchone()
		respone = jsonify(empRow)
		respone.status_code = 200
		return respone
	except Exception as e:
		print(e)
	finally:
		cursor.close() 
		conn.close()

@app.route('/update', methods=['PUT'])
def update_emp():
	try:
		_json = request.json
		_id = _json['id']
		_name = _json['name']
		_email = _json['email']
		_phone = _json['phone']
		_address = _json['address']
		
		validacion = _name and _email and _address and _phone and _id and request.method == 'PUT'

		if validacion:			
			sqlQuery = "UPDATE rest_emp SET name=%s, email=%s, phone=%s, address=%s WHERE id=%s"
			bindData = (_name, _email, _phone, _address, _id,)
			conn = mysql.connect()
			cursor = conn.cursor()
			cursor.execute(sqlQuery, bindData)
			conn.commit()
			respone = jsonify('Employee updated successfully!')
			respone.status_code = 200
			return respone
		else:
			return not_found()	
	except Exception as e:
		print(e)
	finally:
		cursor.close() 
		conn.close()
@app.route('/delete/<int:id>', methods=['DELETE'])
def delete_emp(id):
	try:
		conn = mysql.connect()
		cursor = conn.cursor()
		cursor.execute("DELETE FROM rest_emp WHERE id =%s", (id,))
		conn.commit()
		respone = jsonify('Employee deleted successfully!')
		respone.status_code = 200
		return respone
	except Exception as e:
		print(e)
	finally:
		cursor.close() 
		conn.close()
		
@app.errorhandler(404)
def not_found(error=None):
    message = {
        'status': 404,
        'message': 'Record not found: ' + request.url,
    }
    respone = jsonify(message)
    respone.status_code = 404
    return respone
		
if __name__ == "__main__":
	app.run()