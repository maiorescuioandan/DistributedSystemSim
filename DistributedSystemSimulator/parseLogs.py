import glob, os
import matplotlib.pyplot as plt
import collections

def sorted_ls(path):
    mtime = lambda f: os.stat(os.path.join(path, f)).st_mtime
    return list(sorted(os.listdir(path), key=mtime))

items = sorted_ls(os.getcwd() + os.sep + "Logs" + os.sep)[-1].split('_')
latestFiles = glob.glob(os.getcwd() + os.sep + "Logs" + os.sep + items[0] + "_" + items[1] + "*")

def calculateAverageResponseTime(responseTimeTracker):
    for id in responseTimeTracker.keys():
        responseTimeList = []
        responseTimeTracker[id][kStartTimestamps].sort()
        responseTimeTracker[id][kEndTimestamps].sort()
        for i in range(min(len(responseTimeTracker[id][kStartTimestamps]), len(responseTimeTracker[id][kEndTimestamps]))):
            responseTimeList.append(responseTimeTracker[id][kEndTimestamps][i] - responseTimeTracker[id][kStartTimestamps][i])
        print('Process response timestamp list for process id ' + str(id) +': ' + str(responseTimeTracker[id][kEndTimestamps]))
        print('Process response time list for process id ' + str(id) +': ' + str(responseTimeList))
        plt.ylabel('Response Time for process id ' + str(id))
        plt.xlabel('Node time')
        print(len(responseTimeTracker[id][kEndTimestamps]), len(responseTimeList))
        plt.plot(responseTimeTracker[id][kEndTimestamps], responseTimeList, 'ro')
        plt.axis([0, int(max(responseTimeTracker[id][kEndTimestamps]) * 1.1) + 1, 0, int(max(responseTimeList) * 1.1) + 1])
        plt.show()

def calculateCpuUsage(cpuUsageDict):
    for node in cpuUsageDict.keys():
        orderedDict = collections.OrderedDict(sorted(cpuUsageDict[node].items()))
        xAxis = []
        yAxis = []
        print(str(orderedDict))
        for element in orderedDict:
            xAxis.append(element)
            yAxis.append(orderedDict[element])
        print(str(xAxis))
        print(str(yAxis))
        plt.ylabel('CPU usage for node ' + str(node))
        plt.xlabel('Node time')
        plt.plot(xAxis, yAxis, 'ro')
        plt.axis([0, int(max(xAxis) * 1.1), 0, 1])
        plt.show()
        
def calculateMemUsage(memUsageDict):
    for node in memUsageDict.keys():
        orderedDict = collections.OrderedDict(sorted(memUsageDict[node].items()))
        xAxis = []
        yAxis = []
        print(str(orderedDict))
        for element in orderedDict:
            xAxis.append(element)
            yAxis.append(orderedDict[element])
        print(str(xAxis))
        print(str(yAxis))
        plt.ylabel('Mem usage')
        plt.xlabel('Node time')
        plt.plot(xAxis, yAxis, 'ro')
        plt.axis([0, int(max(xAxis) * 1.1), 0, 1])
        plt.show()
        

nodeMemSizes = {}
memUsage = {}
cpuUsage = {}
responseTimeTracker = {}
kStartTimestamps = 'st'
kEndTimestamps = 'et'
for file in latestFiles:
    fileHandler = open(file, 'r')
    currentNodeId = None
    for line in fileHandler:
        words = line.split()
        if words[0] == 'Created':
            currentNodeId = int(words[4])
            mem = int(words[-1])
            nodeMemSizes[currentNodeId] = mem
            # create dictionary for mem usage on that node
            memUsage[currentNodeId] = {}
            cpuUsage[currentNodeId] = {}
        elif words[0] == 'OK':
            #in this case a new process was created
            #time = int(words[-1])
            id = int(words[4])
            #memReq = int(words[6])
            responseTimeTracker[id] = {kStartTimestamps : [], kEndTimestamps : []}
            #print (memUsage[currentNodeId].keys())
            #if(time in memUsage[currentNodeId].keys()):
            #    memUsage[currentNodeId][time] = memReq + memUsage[currentNodeId][time]
            #else:
            #    memUsage[currentNodeId][time] = memReq
            #print (memUsage[currentNodeId].keys())
        elif words[0] == 'LOOP':
            time = float(words[3])
            id = int(words[-1])
            if words[1] == 'START':
                responseTimeTracker[id][kStartTimestamps].append(time)
            elif words[1] == 'END':
                responseTimeTracker[id][kEndTimestamps].append(time)
        elif words[0] == 'STATUS':
            time = float(words[2])
            cpuUsg = float(words[5])
            memUsg = float(words[-1])
            cpuUsage[currentNodeId][time] = cpuUsg
            memUsage[currentNodeId][time] = memUsg
            
            
print('nodeMemSizes', str(nodeMemSizes))
print('memUsage', str(memUsage))
print('cpuUsage', str(cpuUsage))
print('responseTimeTracker', str(responseTimeTracker))

calculateAverageResponseTime(responseTimeTracker)
calculateCpuUsage(cpuUsage)
calculateMemUsage(memUsage)