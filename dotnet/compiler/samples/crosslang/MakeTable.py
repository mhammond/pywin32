
def MakeTable(gen, param):
    gen.SetParameter(param)
    print gen.GetHeader()
    for col in range(gen.GetColumnCount()):
        for row in range(gen.GetRowCount()):
            print gen.GetCell(row, col)+"\t",
        print
    print gen.GetFooter()

print "--- A Calendar ---"
MakeTable( COR.CalendarGenerator(), "03/1965" )
print "--- A multiplication table ---"
MakeTable( COR.MultiGen.MultiGenerator(), "7*8" )
print "--- A Python Powered Power table ---"
MakeTable( COR.PowerGenerator(), "10*10" )

