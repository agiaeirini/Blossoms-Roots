import RPi.GPIO as GPIO
from time import sleep, strftime
from datetime import datetime
import serial, time
import pymysql.cursors
import smtplib
from email.message import EmailMessage
GPIO.setwarnings(False)
GPIO.setmode(GPIO.BCM) # GPIO Numbers instead of board numbers
Pump = 17
GPIO.setup(Pump, GPIO.OUT) # GPIO Assign mode
GPIO.output(Pump, GPIO.LOW) # out
#GPIO.output(Pump, GPIO.HIGH) # on
sender_email = "*******@gmail.com"
rec_email = "******@gmail.com"
email_message = ""
email = EmailMessage()
email["from"] = ("**********@gmail.com")    #email sender address
email["to"] = "***********@gmail.com"   #email receiver address
email["subject"] = "Trash collection"

email.set_content(email_message)


server = smtplib.SMTP('smtp.gmail.com', 587)

server.starttls()

server.login(sender_email, '*********')
print("Login success")


#change ACM number as found from ===> ls /dev/ttyACM*
microbit=serial.Serial("/dev/ttyACM0")  #change ACM number as found from ===> ls /dev/ttyACM**
microbit.baudrate=115200
#connect to school site database
db=pymysql.connect(host='localhost',user='root',password='1234',db='smartschool')
cursor=db.cursor()
tPump=t=tw=0
sqlcommand = "DELETE FROM `measurements` WHERE `deviceID`<8"
cursor.execute(sqlcommand)
db.commit() 
sql_insert  = "INSERT INTO `measurements`(`datetime`, `deviceID`, `deviceType`, `value`) VALUES (now(),1,'WATT',0)" # wATT CLASS1 
cursor.execute(sql_insert)
db.commit()
sql_insert  = "INSERT INTO `measurements`(`datetime`, `deviceID`, `deviceType`, `value`) VALUES (now(),2,'WATT',0)" # wATT CLASS1 
cursor.execute(sql_insert)
db.commit()
sql_insert  = "INSERT INTO `measurements`(`datetime`, `deviceID`, `deviceType`, `value`) VALUES (now(),3,'BIKE',0)" # wATT CLASS1 
cursor.execute(sql_insert)
db.commit()
sql_insert  = "INSERT INTO `measurements`(`datetime`, `deviceID`, `deviceType`, `value`) VALUES (now(),4,'TREADMILL',0)" # wATT CLASS1 
cursor.execute(sql_insert)
db.commit()

#set GPIO Pins
GPIO_TRIGGER = 23
GPIO_ECHO = 24
#set GPIO direction (IN / OUT)
GPIO.setup(GPIO_TRIGGER, GPIO.OUT)
GPIO.setup(GPIO_ECHO, GPIO.IN)
GPIO.setup(25,GPIO.OUT)
GPIO.output(25,GPIO.LOW)

 
def distance():
    # set Trigger to HIGH
    GPIO.output(GPIO_TRIGGER, True)
    # set Trigger after 0.01ms to LOW
    time.sleep(0.00001)
    GPIO.output(GPIO_TRIGGER, False)
    StartTime = time.time()
    StopTime = time.time()
     # save StartTime
    while GPIO.input(GPIO_ECHO) == 0:
        StartTime = time.time()
     # save time of arrival
    while GPIO.input(GPIO_ECHO) == 1:
        StopTime = time.time()
     # time difference between start and arrival
    TimeElapsed = StopTime - StartTime
    # multiply with the sonic speed (34300 cm/s)
    # and divide by 2, because there and back
    distance = (TimeElapsed * 34300) / 2
    return distance


def Select_Watt(deviceID,deviceType):
    sql ="SELECT value FROM measurements where deviceID= "+deviceID+" and `deviceType`='"+deviceType+"' order by datetime desc limit 1"
    cursor.execute(sql)
    w = cursor.fetchall()
    w =str(w)
    w =w[3:]
    w_len=len(str(w))
    w=w[:w_len-5]
    return w
    #print(w)




try:
    while True:
        dist = distance()
        
        if dist<4:
            GPIO.output(25,GPIO.HIGH) #led on
            print ("Measured Distance = %.1f cm" % dist)
            server.send_message(email)

            print("Email has been sent to ", rec_email)
        else:
            GPIO.output(25,GPIO.LOW) #led oFF
            
        #print ("tw:",tw)
        t=t+1
        tw=tw+1
        tPump=tPump+1
        #read date from microbit over serial port
        microbit_data = microbit.readline()
        microbit_data_str=str(microbit_data)      
        microbit_data_str=microbit_data_str[2:]
        microbit_data_len=len(str(microbit_data_str))
        microbit_data_str=microbit_data_str[:microbit_data_len-5]
        #data = int(data[0:4])
        #print(len(str(microbit_data_str)))            
        print(str(microbit_data_str))
        if (microbit_data_str !="None" and (microbit_data_str[0]=="2" or microbit_data_str[0]=="1" or microbit_data_str[0]=="6")): #if str begin with 1 or 2
            #build the INSERT statement to write values to the  database
            sqlcommand = "INSERT INTO `smartschool`.`measurements` (`datetime`, `deviceID`, `deviceType`, `value`) VALUES (now(),"+microbit_data_str+")"
            #print(sqlcommand)

            #execute the INSERT statement
            cursor.execute(sqlcommand)


         
           
            #commit the insertions to the database
            db.commit()           

           
            if tw>10:
                w1=Select_Watt("1","WATT")
                w2=Select_Watt("2","WATT")
                w3=Select_Watt("3","BIKE")
                w4=Select_Watt("4","Treadmill")
                     #calculate Sum Watt +Sun_Watt=4
                Sw=float(w3)+float(w4)-float(w1)-float(w2) +4.0
                Sw=round(Sw,2)
                    #print(w1) 
                sqlcommand ="INSERT INTO `measurements`(`datetime`, `deviceID`, `deviceType`, `value`) VALUES (now(),5,'Sw',"+str(Sw)+")"
                cursor.execute(sqlcommand)
                print("SumWatt=",Sw)
                tw=0
                
            
            if (tPump<115):
                GPIO.output(Pump, GPIO.LOW) #κλειστή η αντλία
            else :
                GPIO.output(Pump, GPIO.HIGH) #ανοιχτή η αντλία
            if (tPump==150):
                tPump=0
except Exception as ex:
    print(ex)
    GPIO.cleanup()

finally:
    cursor.close()
    db.close()
    GPIO.cleanup()

