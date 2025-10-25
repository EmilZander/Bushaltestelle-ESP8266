import requests
import json

url = "https://www.vrs.de/index.php?eID=tx_vrsinfo_departuremonitor&i=a3b651c91bcb80ac1fe595b18bed4ffb"

jsonStr = requests.get(url).text

jsonFile = json.loads(jsonStr)

displayText = ""
for i in range(2):
    time = jsonFile["events"][i]["departure"]["timetable"]
    line = jsonFile["events"][i]["line"]["number"]
    direction = jsonFile["events"][i]["line"]["direction"]

    text = time+"|"+line+"|"+direction
    displayText += text+"\n"

print(displayText)