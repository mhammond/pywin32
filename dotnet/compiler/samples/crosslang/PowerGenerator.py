class PowerGenerator(COR.TableGen.TableGenerator):

    # SetParameter - sets the max rows and cols for our calculator.
    def SetParameter(self, param):
        # Get the rows and cols from the param.
        rows, cols = param.split("*", 2)
        self.rows = int(rows)
        self.cols = int(cols)
        return 1

    def GetColumnCount(self):
        return self.cols

    def GetRowCount(self):
        return self.rows

    def GetHeader(self):
        # Returns: String: HTML header for the top of the table.
        # Like the VB sample, we embed the header in the cells.
        return ''

    def GetFooter(self):
        return ''

    def GetCell(self, col, row):
        if col==0 and row==0:
            return ''
        if col==0 or row==0:
            if col==0:
                val = row
            else:
                val = col
            return "<B>" + str(val) + "</B>"
        return str((col+1) ** (row+1))
