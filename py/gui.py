from PyQt5 import QtWidgets, QtCore, QtGui
from PyQt5.QtWidgets import QLabel

from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtGui import *
import os
import subprocess
import PyQt5


class GUI(QtWidgets.QMainWindow):

    def __init__(self,card_numbers,card_holder,AID,Key,application_label, priorities, tracks,expirations,file):
        self.card_numbers = card_numbers
        self.AID = AID
        self.Key = Key
        self.card_holder = card_holder
        self.tracks = tracks
        self.application_label = application_label
        self.priorities = priorities
        self.expirations = expirations
        self.file = file
        self.dump = self.file.read()
        super().__init__()
        self.setCentralWidget(QtWidgets.QWidget())  # QMainWindown must have a centralWidget to be able to add layouts
        self.layout = QtWidgets.QVBoxLayout()  # Vertical main layout(s)
        self.centralWidget().setLayout(self.layout)  # set the main layout as the central widget
        self.vbox = QVBoxLayout()
        self.top_widget = QtWidgets.QWidget()
        self.DNA_top_layout = QtWidgets.QHBoxLayout()
        self.top_widget.setLayout(self.DNA_top_layout)
        self.init_window()
        self.add_text()

        self.button()






    def init_window(self): #sets up the main window

        self.setGeometry(300, 300, 800, 800)

        self.showMaximized()
        # Add scenes for drawing 2d objects
        self.scene_top = QtWidgets.QGraphicsScene()


        self.view_top = QtWidgets.QGraphicsView(self.scene_top, self)
        self.view_top.setAlignment(QtCore.Qt.AlignTop)
        self.view_top.setAlignment(QtCore.Qt.AlignBottom)

        self.view_top.show()

        self.layout.addWidget(self.view_top)




    def add_text(self): #adds various texts to the view and the windowtitle
        font=13
        x=-0
        y_list=[-400,-300,-200,-100,0,100,200,300,400,500,600,700]
        # for y in range(-400,300,100):
        #     self.scene_top.addLine(0,y,800,y)

        self.dump_label = QLabel(str(self.dump))
        self.dump_label.setFixedSize(500, 1600)
        self.dump_label.setFont(QFont('Arial', 10))
        self.dump_label.move(850, -400)


        self.setWindowTitle('Gimme ya wallet')
        # text

        self.AID_label = QLabel("AID:"+str(self.AID))
        self.AID_label.move(x, y_list[0])
        self.AID_label.setFont(QFont("Arial", font))
        self.AID_label.setStyleSheet("border :3px solid blue;")




        self.key= QLabel(" Issuer Public KeyCertificate:"+str(self.Key))
        self.key.setFont(QFont("Arial", font))
        self.key.move(x, y_list[1])
        self.key.setStyleSheet("border :3px solid black;")


        self.Cardholder_Name_label = QLabel("Cardholder Name:"+str(self.card_holder))
        self.Cardholder_Name_label.setFont(QFont("Arial", font))
        self.Cardholder_Name_label.move(x, y_list[2])
        self.Cardholder_Name_label.setStyleSheet("border :3px solid blue;")

        self.Cardnumber_label = QLabel("Cardnumber/PAN:" + str(self.card_numbers))
        self.Cardnumber_label.setFont(QFont("Arial", font))
        self.Cardnumber_label.move(x, y_list[7])
        self.Cardnumber_label.setStyleSheet("border :3px solid blue;")


        self.track_label = QLabel("Track 1 DiscretionaryData:"+str(self.tracks[0])+"\nTrack 2 Equivalent Data:"+str(self.tracks[0]))
        self.track_label.setFont(QFont("Arial", font))
        self.track_label.move(x, y_list[3])
        self.track_label.setStyleSheet("border :3px solid black;")


        self.track2 = QLabel("Priority Indicator:"+str(self.priorities))
        self.track2.setFont(QFont("Arial", font))
        self.track2.move(x, y_list[4])
        self.track2.setStyleSheet("border :3px solid blue;")


        self.expiration_label = QLabel("Application Expiration Date:" + str(self.expirations))
        self.expiration_label.setFont(QFont("Arial", font))
        self.expiration_label.move(x, y_list[5])
        self.expiration_label.setStyleSheet("border :3px solid blue;")

        self.application_label2 = QLabel("Application Label:" + str(self.application_label))
        self.application_label2.setFont(QFont("Arial", font))
        self.application_label2.move( x, y_list[6])
        self.application_label2.setStyleSheet("border :3px solid blue;")

        self.scene_top.addWidget(self.AID_label)
        self.scene_top.addWidget(self.key)
        self.scene_top.addWidget(self.Cardholder_Name_label)
        self.scene_top.addWidget(self.track_label)
        self.scene_top.addWidget(self.track2)
        self.scene_top.addWidget(self.expiration_label)
        self.scene_top.addWidget(self.Cardnumber_label)
        self.scene_top.addWidget(self.application_label2)
        self.scene_top.addWidget(self.dump_label)

    def executor(self):  # executes the C++ scripts. And python regex script. Gives data file to this python script to read and display
        proc = subprocess.Popen(["g++ ./rdr/rdr.cpp -std=c++17 -fpermissive -lnfc -o ./rdr/rdr | ./rdr/rdr"],
                                stdout=subprocess.PIPE, shell=True)
        proc.wait()
        proc = subprocess.Popen(["python3 ./py/parser.py"])
        proc.wait()
        # (out, err) = proc.communicate()




    def button(self):


        self.button = QPushButton('EXECUTE HACK', self)
        self.button.move(50, 50)
        self.button.clicked.connect(lambda: self.executor())
        self.layout.addWidget(self.button)





