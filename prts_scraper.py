from bs4 import BeautifulSoup
import urllib.request
import traceback

def scrape(name, voiceType, outdir):
    print("downloading " + name)
    escapeName = urllib.parse.quote(name)
    escapeVoiceType = urllib.parse.quote(voiceType)
    with urllib.request.urlopen("http://prts.wiki/w/" + escapeName) as f:
        data = f.read().decode("utf-8")
        soup = BeautifulSoup(data)
        key = soup.find("div", {"id" : "voice-data-root"}).attrs["data-voice-key"]
        if outdir[-1] != '/':
            outdir += "/"
        fileName = outdir + name + "_" + voiceType + ".wav"
        resUrl = "https://static.prts.wiki/voice/" + key + "/" + escapeName + "_" + escapeVoiceType + ".wav"
        urllib.request.urlretrieve(resUrl, fileName)

def scrapeAll(names, voiceType, outdir):
    if type(names) == str:
        names = names.replace(',',' ').split()
    for name in names:
        try:
            scrape(name, voiceType, outdir)
        except Exception as e:
            traceback.print_exc()


        
