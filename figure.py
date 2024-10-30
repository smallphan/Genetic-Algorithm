import pandas as pd
import matplotlib.pyplot as plt

def ReadSequence(filePath: str):
    with open(filePath, 'r') as file:
        data = file.read().strip().split(', ')
        return [float(num) for num in data]

def Figure():
    dataA = ReadSequence("./result/NGA.seq")
    dataB = ReadSequence("./result/IGA.seq")

    dfA = pd.DataFrame(dataA, columns=['NGA'] )
    dfB = pd.DataFrame(dataB, columns=['IGA'])

    plt.figure(figsize=(10, 5))
    plt.plot(dfA['NGA'], marker='x', label='Sequence NGA')
    plt.plot(dfB['IGA'], marker='o', label='Sequence IGA')
    plt.title('Sequence')
    plt.xlabel('Generation')
    plt.ylabel('Target')
    plt.grid()
    plt.legend()  
    plt.show()


Figure()
