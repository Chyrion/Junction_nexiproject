from PyQt5 import QtWidgets, QtCore, QtGui
from PyQt5.QtWidgets import QLabel

from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtGui import *
import PyQt5


class GUI(QtWidgets.QMainWindow):

    def __init__(self,card_numbers,card_holder,AID,Key,application_label, priorities, tracks):
        self.card_numbers=card_numbers
        self.AID = AID
        self.Key = Key
        self.card_holder = card_holder
        self.tracks = tracks
        self.application_label = application_label
        self.priorities = priorities
        super().__init__()
        self.setCentralWidget(QtWidgets.QWidget())  # QMainWindown must have a centralWidget to be able to add layouts
        self.layout = QtWidgets.QVBoxLayout()  # Vertical main layout(s)
        self.centralWidget().setLayout(self.layout) #set the main layout as the central widget
        self.vbox = QVBoxLayout()
        self.top_widget=QtWidgets.QWidget()
        self.DNA_top_layout = QtWidgets.QHBoxLayout()
        self.top_widget.setLayout(self.DNA_top_layout)
        self.init_window()
        self.add_text()
        self.file_name=None




        #self.file_prompt() #calls the file prompt function at the init


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
        font=15
        x=-0
        y_list=[-400,-300,-200,-100,0,100,200,300,400,500,600,700]
        # for y in range(-400,300,100):
        #     self.scene_top.addLine(0,y,800,y)


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


        self.TLV_data_d = QLabel("Decoded TLV_data placeholder:" + str(self.application_label))
        self.TLV_data_d.setFont(QFont("Arial", font))
        self.TLV_data_d.move(x, y_list[6])
        self.TLV_data_d.setStyleSheet("border :3px solid blue;")

        self.application_label2 = QLabel("Application Label:" + str(self.application_label))
        self.application_label2.setFont(QFont("Arial", font))
        self.application_label2.move( x, y_list[6])
        self.application_label2.setStyleSheet("border :3px solid blue;")

        self.scene_top.addWidget(self.AID_label)
        self.scene_top.addWidget(self.key)
        self.scene_top.addWidget(self.Cardholder_Name_label)
        self.scene_top.addWidget(self.track_label)
        self.scene_top.addWidget(self.track2)
        self.scene_top.addWidget(self.TLV_data_d)
        self.scene_top.addWidget(self.Cardnumber_label)
        self.scene_top.addWidget(self.application_label2)





