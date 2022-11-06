import sys
from gui import *
import os
import subprocess
#import parser #this script handles formatting the TLV data     my script crashes on my system when this isnt commented out
import re
def executor(): #executes the C++ scripts. And python regex script. Gives data file to this python script to read and display
    proc = subprocess.Popen(["g++ ./rdr/rdr.cpp -std=c++17 -fpermissive -lnfc -o ./rdr/rdr | ./rdr/rdr"], stdout=subprocess.PIPE, shell=True)
    proc.wait()
    proc = subprocess.Popen(["python3 ./py/parser.py"])
    proc.wait()
    #(out, err) = proc.communicate()

    pass

def reader1(file):
    with open(file, "r") as f:
        data = f.read()
        data = data.splitlines()
        print(data)



        def finder(data, label):
            finder_index_list = []
            index = 0
            for i in data:
                if i == label:
                    finder_index_list.append(index)
                index += 1
            return finder_index_list

        AID_index=finder(data, "Application Identifier (AID)  card")
        AIDS=[]
        for i in AID_index:
            AIDS.append(data[i+1])


        key_index=finder(data, "Issuer Public Key Certificate")
        keys=[]
        for key in key_index:
            keys.append(data[key-1])
        if len(keys)==0:
            keys.append("No Key Found")




        card_holder_index=finder(data, "Cardholder Name")
        card_holders=[]
        for card_holder in card_holder_index:
            card_holders.append(data[card_holder+1])
        if len(card_holders)==0:
            card_holders.append("No Card Holder Found")



        application_label_index=finder(data, "Application Label")
        application_labels = []
        for application in application_label_index:
            application_labels.append(data[application+1])
        if len(application_labels)==0:
            application_labels.append("No Application Label Found")

        #return AIDS,keys,card_holders,application_labels

        priority_index=finder(data, "Priority Indicator")
        priorities=[]
        for priority in priority_index:
            priorities.append(data[priority+1])
        if len(priorities)==0:
            priorities.append("No Priority Found")

        card_number_index=finder(data, "Card Number")
        card_numbers=[]
        for card_number in card_number_index:
            card_numbers.append(data[card_number+1])
        if len(card_numbers)==0:
            card_numbers.append("No Card Number Found")

        tracks_index=finder(data, "Track 1 Data")
        tracks=[]
        for track in tracks_index:
            tracks.append(data[track+1])
        if len(tracks)==0:
            tracks.append("No Track Data Found")

        expiration_index = finder(data, "Application Expiration Date")
        expirations = []
        for expiration in expiration_index:
            expirations.append(data[expiration + 1])
        if len(expirations) == 0:
            expirations.append("No Expiration Date Found")

        return card_numbers,card_holders,AIDS,keys,application_labels,priorities, tracks, expirations, file





        #magstripe data entry





def main():
    #print(reader1("output.txt"))
    # Start the Qt event loop. (i.e. make it possible to interact with the gui)
    executor()
    global app  # Use global to prevent crashing on exit
    app = QApplication(sys.argv)

    file="output.txt"

    final_list=reader1(file)
    #dict=reader()
    #gui = GUI(dict[AID],dict[key],dict[card_holder],dict[track1],dict[track2])
    #print(final_list)
    #gui = GUI(final_list[0],final_list[1],final_list[2],"track1","track2",final_list[3])
    print(final_list)
    gui=GUI(final_list[0],final_list[1],final_list[2],final_list[3], final_list[4], final_list[5], final_list[6], final_list[7], open(file, "r"))
    gui.show()
    sys.exit(app.exec_())


    sys.exit(app.exec_())




if __name__ == "__main__":
    main()