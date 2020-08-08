from app import app
from flaskext.mysql import MySQL

mysql = MySQL()
app.config['MYSQL_DATABASE_USER'] = 'developer'
app.config['MYSQL_DATABASE_PASSWORD'] = 'D3v3l0p@r'
app.config['MYSQL_DATABASE_DB'] = 'ProyectoIII'
app.config['MYSQL_DATABASE_HOST'] = 'localhost'
mysql.init_app(app)