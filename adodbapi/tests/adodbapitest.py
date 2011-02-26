""" Unit tests for adodbapi version 2.4.2"""
"""
    adodbapi - A python DB API 2.0 interface to Microsoft ADO
    
    Copyright (C) 2002  Henrik Ekelund

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Updates by Vernon Cole
"""

import unittest
import sys

try:
    import win32com.client
    win32 = True
except ImportError:
    win32 = False

try:
    import adodbapitestconfig as config #will find (parent?) adodbapi
except ImportError:
    import testADOdbapiConfig as config #alternate version
import adodbapi
try:
    import adodbapi.ado_consts as ado_consts
except ImportError: #we are doing a shortcut import as a module -- so
    try:
        import ado_consts
    except ImportError:
        from adodbapi import ado_consts
    
import types
try:
    import decimal
except ImportError:
    import win32com.decimal_23 as decimal

def str2bytes(sval):
    if sys.version_info < (3,0) and isinstance(sval, str):
        sval = sval.decode("latin1")
    return sval.encode("latin1")

class CommonDBTests(unittest.TestCase):
    "Self contained super-simple tests in easy syntax, should work on everything between mySQL and Oracle"

    def setUp(self):
        self.engine = 'unknown'

    def getEngine(self):
        return self.engine
    
    def getConnection(self):
        raise NotImplementedError #"This method must be overriden by a subclass"  

    def getCursor(self):
        return self.getConnection().cursor()

    def testConnection(self):
        crsr=self.getCursor()
        assert crsr.__class__.__name__ == 'Cursor'

    def testErrorHandlerInherits(self):
        conn=self.getConnection()
        mycallable=lambda connection,cursor,errorclass,errorvalue: 1
        conn.errorhandler=mycallable
        crsr=conn.cursor()
        assert crsr.errorhandler==mycallable,"Error handler on crsr should be same as on connection"

    def testDefaultErrorHandlerConnection(self):
        conn=self.getConnection()
        del conn.messages[:]
        try:
            conn.close()
            conn.commit() #Should not be able to use connection after it is closed
        except:
            assert len(conn.messages)==1
            assert len(conn.messages[0])==2
            assert conn.messages[0][0]==adodbapi.Error
            
    def testOwnErrorHandlerConnection(self):
        mycallable=lambda connection,cursor,errorclass,errorvalue: 1 #does not raise anything
        conn=self.getConnection()        
        conn.errorhandler=mycallable
        conn.close()
        conn.commit() #Should not be able to use connection after it is closed        
        assert len(conn.messages)==0
       
        conn.errorhandler=None #This should bring back the standard error handler
        try:
            conn.close()
            conn.commit() #Should not be able to use connection after it is closed
        except:
            pass
        #The Standard errorhandler appends error to messages attribute
        assert len(conn.messages)>0,"Setting errorhandler to none  should bring back the standard error handler"


    def testDefaultErrorHandlerCursor(self):
        crsr=self.getConnection().cursor()
        del crsr.messages[:]       
        try:
            crsr.execute("SELECT abbtytddrf FROM dasdasd")
        except:
            assert len(crsr.messages)==1
            assert len(crsr.messages[0])==2
            assert crsr.messages[0][0]==adodbapi.DatabaseError
            
    def testOwnErrorHandlerCursor(self):
        mycallable=lambda connection,cursor,errorclass,errorvalue: 1 #does not raise anything
        crsr=self.getConnection().cursor()
        crsr.errorhandler=mycallable
        crsr.execute("SELECT abbtytddrf FROM dasdasd")
        assert len(crsr.messages)==0
        
        crsr.errorhandler=None #This should bring back the standard error handler
        try:
            crsr.execute("SELECT abbtytddrf FROM dasdasd")
        except:
            pass
        #The Standard errorhandler appends error to messages attribute
        assert len(crsr.messages)>0,"Setting errorhandler to none  should bring back the standard error handler"


    def testUserDefinedConversions(self):  
        oldconverter=adodbapi.variantConversions[adodbapi.adoStringTypes]
#try:
        duplicatingConverter=lambda aStringField: aStringField*2
        assert duplicatingConverter(u'gabba') == u'gabbagabba'

        # change converter for ALL adoStringTypes columns
        adodbapi.variantConversions[adodbapi.adoStringTypes]=duplicatingConverter

        self.helpForceDropOnTblTemp()
        conn=self.getConnection()

        crsr=conn.cursor()
        tabdef = "CREATE TABLE tblTemp (fldData VARCHAR(100) NOT NULL, fld2 VARCHAR(20))"
        crsr.execute(tabdef)
        crsr.execute("INSERT INTO tblTemp(fldData,fld2) VALUES('gabba','booga')")
        crsr.execute("INSERT INTO tblTemp(fldData,fld2) VALUES('hey','yo')")
        crsr.execute("SELECT fldData,fld2 FROM tblTemp ORDER BY fldData")
        rows=crsr.fetchall()
        row = rows[0]
        self.assertEquals(row[0],'gabbagabba')
        row = rows[1]
        self.assertEquals(row[0],'heyhey')                     
        self.assertEquals(row[1],'yoyo')

        upcaseConverter=lambda aStringField: aStringField.upper()
        assert upcaseConverter(u'upThis') == u'UPTHIS'

        # now use a single column converter
        rows.converters[1] = upcaseConverter  # convert second column
        self.assertEquals(row[0],'heyhey')    # first will be unchanged
        self.assertEquals(row[1],'YO')        # second will convert to upper case
        
#finally:
        adodbapi.variantConversions[adodbapi.adoStringTypes]=oldconverter #Restore
        self.helpRollbackTblTemp()

    def helpTestDataType(self,sqlDataTypeString,
                         DBAPIDataTypeString,
                         pyData,
                         pyDataInputAlternatives=None,
                         compareAlmostEqual=None,
                         allowedReturnValues=None):
        self.helpForceDropOnTblTemp()
        conn=self.getConnection()       
        crsr=conn.cursor()
        tabdef= """
            CREATE TABLE tblTemp (
                fldId integer NOT NULL,
                fldData """ + sqlDataTypeString + ")\n"

        crsr.execute(tabdef)
        
        #Test Null values mapped to None
        crsr.execute("INSERT INTO tblTemp (fldId) VALUES (1)")
        
        crsr.execute("SELECT fldId,fldData FROM tblTemp")
        rs=crsr.fetchone()
        self.assertEquals(rs[1],None) #Null should be mapped to None
        assert rs[0]==1

        #Test description related 
        descTuple=crsr.description[1]
        assert descTuple[0] in ['fldData','flddata'], 'was "%s" expected "%s"'%(descTuple[0],'fldData')

        if DBAPIDataTypeString=='STRING':
            assert descTuple[1] == adodbapi.STRING, 'was "%s" expected "%s"'%(descTuple[1],adodbapi.STRING.values)
        elif DBAPIDataTypeString == 'NUMBER':
            assert descTuple[1] == adodbapi.NUMBER, 'was "%s" expected "%s"'%(descTuple[1],adodbapi.NUMBER.values)
        elif DBAPIDataTypeString == 'BINARY':
            assert descTuple[1] == adodbapi.BINARY, 'was "%s" expected "%s"'%(descTuple[1],adodbapi.BINARY.values)
        elif DBAPIDataTypeString == 'DATETIME':
            assert descTuple[1] == adodbapi.DATETIME, 'was "%s" expected "%s"'%(descTuple[1],adodbapi.DATETIME.values)
        elif DBAPIDataTypeString == 'ROWID':
            assert descTuple[1] == adodbapi.ROWID, 'was "%s" expected "%s"'%(descTuple[1],adodbapi.ROWID.values)
        else:
            raise NotImplementedError #"DBAPIDataTypeString not provided"

        #Test data binding
        inputs=[pyData]
        if pyDataInputAlternatives:
            inputs.append(pyDataInputAlternatives)

        fldId=1
        for inParam in inputs:
            fldId+=1
            try:
                crsr.execute("INSERT INTO tblTemp (fldId,fldData) VALUES (?,?)", (fldId,pyData))
            except:
                conn.printADOerrors()
                raise
            crsr.execute("SELECT fldData FROM tblTemp WHERE ?=fldID", [fldId])
            rs=crsr.fetchone()
            if allowedReturnValues:
                allowedTypes = tuple([type(aRV) for aRV in allowedReturnValues])
                assert isinstance(rs[0],allowedTypes), \
                       'result type "%s" must be one of %s'%(type(rs[0]),allowedTypes)
            else:
                assert isinstance(rs[0] ,type(pyData)), \
                       'result type "%s" must be instance of %s'%(type(rs[0]),type(pyData))

            if compareAlmostEqual and DBAPIDataTypeString == 'DATETIME':
                iso1=adodbapi.dateconverter.DateObjectToIsoFormatString(rs[0])
                iso2=adodbapi.dateconverter.DateObjectToIsoFormatString(pyData)
                self.assertEquals(iso1 , iso2)
            elif compareAlmostEqual:
                s = float(pyData)
                v = float(rs[0])
                assert abs(v-s)/s < 0.00001, \
                    "Values not almost equal recvd=%s, expected=%f" %(rs[0],s)
            else:
                if allowedReturnValues:
                    ok=False
                    for possibility in allowedReturnValues:
                        if rs[0]==possibility:
                            ok=True
                    assert ok
                else:                
                    self.assertEquals(rs[0] , pyData, \
                        'Values are not equal recvd="%s", expected="%s"' %(rs[0],pyData))

    def testDataTypeFloat(self):       
        self.helpTestDataType("real",'NUMBER',3.45,compareAlmostEqual=True)
        self.helpTestDataType("float",'NUMBER',1.79e37,compareAlmostEqual=True)

    def testDataTypeMoney(self):    #v2.1 Cole -- use decimal for money
        if self.getEngine() == 'MySQL':
            pass
        elif self.getEngine() == 'PostgreSQL':
            self.helpTestDataType("money",'NUMBER',decimal.Decimal('-922337203685477.5808'),
                                  compareAlmostEqual=True,
                                  allowedReturnValues=[-922337203685477.5808,
                                                       decimal.Decimal('-922337203685477.5808')])
        else:
            self.helpTestDataType("smallmoney",'NUMBER',decimal.Decimal('214748.02'))        
            self.helpTestDataType("money",'NUMBER',decimal.Decimal('-922337203685477.5808'))

    def testDataTypeInt(self):
        if self.getEngine() != 'PostgreSQL':
            self.helpTestDataType("tinyint",'NUMBER',115)
        self.helpTestDataType("smallint",'NUMBER',-32768)
        self.helpTestDataType("int",'NUMBER',2147483647,
                              pyDataInputAlternatives='2137483647')
        if self.getEngine() not in ['ACCESS','PostgreSQL']:
            self.helpTestDataType("bit",'NUMBER',1) #Does not work correctly with access        
        if self.getEngine() != 'ACCESS':
            self.helpTestDataType("bigint",'NUMBER',3000000000) 

    def testDataTypeChar(self):
        for sqlDataType in ("char(6)","nchar(6)"):
            self.helpTestDataType(sqlDataType,'STRING',u'spam  ',allowedReturnValues=[u'spam','spam',u'spam  ','spam  '])

    def testDataTypeVarChar(self):
        if self.getEngine() == 'MySQL':
            stringKinds = ["varchar(10)","text"]
        elif self.getEngine() == 'PostgreSQL':
            stringKinds = ["varchar(10)","text","character varying"]
        else:
            stringKinds = ["varchar(10)","nvarchar(10)","text","ntext"] #,"varchar(max)"]

        for sqlDataType in stringKinds:
            self.helpTestDataType(sqlDataType,'STRING',u'spam',['spam'])
            
    def testDataTypeDate(self):
        if self.getEngine() == 'PostgreSQL':
            dt = "timestamp"
        else:
            dt = "datetime"
        self.helpTestDataType(dt,'DATETIME',adodbapi.Date(2002,10,28),compareAlmostEqual=True)
        if self.getEngine() not in ['MySQL','PostgreSQL']:
            self.helpTestDataType("smalldatetime",'DATETIME',adodbapi.Date(2002,10,28),compareAlmostEqual=True)
        if self.getEngine() != 'PostgreSQL': # fails when using pythonTime
            self.helpTestDataType(dt,'DATETIME',adodbapi.Timestamp(2002,10,28,12,15,1),compareAlmostEqual=True)

    def testDataTypeBinary(self):
        binfld = str2bytes('\x00\x01\xE2\x40')
        if self.getEngine() == 'MySQL':
            pass #self.helpTestDataType("BLOB",'BINARY',adodbapi.Binary(binfld))
        elif self.getEngine() == 'PostgreSQL':
            self.helpTestDataType("bytea",'BINARY',adodbapi.Binary(binfld))
        else:
            self.helpTestDataType("binary(4)",'BINARY',adodbapi.Binary(binfld))
            self.helpTestDataType("varbinary(100)",'BINARY',adodbapi.Binary(binfld))
            self.helpTestDataType("image",'BINARY',adodbapi.Binary(binfld))

    def helpRollbackTblTemp(self):
        try:
            self.getConnection().rollback()
        except adodbapi.NotSupportedError:
            pass
        self.helpForceDropOnTblTemp()
        
    def helpForceDropOnTblTemp(self):
        conn=self.getConnection()
        crsr=conn.cursor()
        try:
            crsr.execute("DELETE FROM tblTemp")
            crsr.execute("DROP TABLE tblTemp")
            conn.commit()
        except:
            pass
        #finally:
        crsr.close()

    def helpCreateAndPopulateTableTemp(self,crsr):
        self.helpForceDropOnTblTemp()
        tabdef= """
            CREATE TABLE tblTemp (
                fldData INTEGER
            )
            """
        crsr.execute(tabdef)
        for i in range(9): # note: this poor SQL code, but a valid test
            crsr.execute("INSERT INTO tblTemp (fldData) VALUES (%i)" %(i,))
            # better to use ("INSERT INTO tblTemp (fldData) VALUES (?)",(i,))
            
    def testFetchAll(self):      
        crsr=self.getCursor()
        self.helpCreateAndPopulateTableTemp(crsr)
        crsr.execute("SELECT fldData FROM tblTemp")
        rs=crsr.fetchall()
        assert len(rs)==9
        #test slice of rows
        i = 3
        for row in rs[3:-2]: #should have rowid 3..6
            assert row[0]==i
            i+=1
        self.helpRollbackTblTemp()

    def testIterator(self):      
        crsr=self.getCursor()
        self.helpCreateAndPopulateTableTemp(crsr)
        crsr.execute("SELECT fldData FROM tblTemp")
        for i,row in enumerate(crsr): # using cursor rather than fetchxxx
            assert row[0]==i
        self.helpRollbackTblTemp()
        
    def testExecuteMany(self):
        crsr=self.getCursor()
        self.helpCreateAndPopulateTableTemp(crsr)
        values = [ (111,) , (222,) ]
        crsr.executemany("INSERT INTO tblTemp (fldData) VALUES (?)",values)
        if crsr.rowcount==-1:
            print self.getEngine(),"Provider does not support rowcount (on .executemany())"
        else:
            self.assertEquals( crsr.rowcount,2)
        crsr.execute("SELECT fldData FROM tblTemp")
        rs=crsr.fetchall()
        assert len(rs)==11
        self.helpRollbackTblTemp()
        

    def testRowCount(self):      
        crsr=self.getCursor()
        self.helpCreateAndPopulateTableTemp(crsr)
        crsr.execute("SELECT fldData FROM tblTemp")
        if crsr.rowcount == -1:
            #print "provider does not support rowcount on select"
            pass
        else:
            self.assertEquals( crsr.rowcount,9)
        self.helpRollbackTblTemp()
        
    def testRowCountNoRecordset(self):      
        crsr=self.getCursor()
        self.helpCreateAndPopulateTableTemp(crsr)
        crsr.execute("DELETE FROM tblTemp WHERE fldData >= 5")
        if crsr.rowcount==-1:
            print self.getEngine(), "Provider does not support rowcount (on DELETE)"
        else:
            self.assertEquals( crsr.rowcount,4)
        self.helpRollbackTblTemp()        
        
    def testFetchMany(self):
        crsr=self.getCursor()
        self.helpCreateAndPopulateTableTemp(crsr)
        crsr.execute("SELECT fldData FROM tblTemp")
        rs=crsr.fetchmany(3)
        assert len(rs)==3
        rs=crsr.fetchmany(5)
        assert len(rs)==5
        rs=crsr.fetchmany(5)
        assert len(rs)==1 #Ask for five, but there is only one left
        self.helpRollbackTblTemp()        

    def testFetchManyWithArraySize(self):
        crsr=self.getCursor()
        self.helpCreateAndPopulateTableTemp(crsr)
        crsr.execute("SELECT fldData FROM tblTemp")
        rs=crsr.fetchmany()
        assert len(rs)==1 #arraysize Defaults to one
        crsr.arraysize=4
        rs=crsr.fetchmany()
        assert len(rs)==4
        rs=crsr.fetchmany()
        assert len(rs)==4
        rs=crsr.fetchmany()
        assert len(rs)==0 
        self.helpRollbackTblTemp()

    def testErrorConnect(self):
        self.assertRaises(adodbapi.DatabaseError,adodbapi.connect,'not a valid connect string')

    def testRowIterator(self):
        self.helpForceDropOnTblTemp()
        conn=self.getConnection()
        crsr=conn.cursor()
        tabdef= """
            CREATE TABLE tblTemp (
                fldId integer NOT NULL,
                fldTwo integer,
                fldThree integer,
                fldFour integer)
                """
        crsr.execute(tabdef)

        inputs = [(2,3,4),(102,103,104)]
        fldId=1
        for inParam in inputs:
            fldId+=1
            try:
                crsr.execute("INSERT INTO tblTemp (fldId,fldTwo,fldThree,fldFour) VALUES (?,?,?,?)", (fldId,inParam[0],inParam[1],inParam[2]))
            except:
                conn.printADOerrors()
                raise
            crsr.execute("SELECT fldTwo,fldThree,fldFour FROM tblTemp WHERE ?=fldID",[fldId])
            rec = crsr.fetchone()
            # check that stepping through an emulated row works
            for j in range(len(inParam)):
                assert rec[j] == inParam[j], 'returned value:"%s" != test value:"%s"'%(rec[j],inParam[j])
            # check that we can get a complete tuple from a row
            assert tuple(rec) == inParam, 'returned value:"%s" != test value:"%s"'%(repr(rec),repr(inParam))
            # test that slices of rows work
            slice1 = tuple(rec[:-1])
            slice2 = tuple(inParam[0:2])
            assert slice1 == slice2, 'returned value:"%s" != test value:"%s"'%(repr(slice1),repr(slice2))
            # now test named column retrieval
            assert rec['fldTwo'] == inParam[0]
            assert rec.fldThree == inParam[1]
            assert rec.fldFour == inParam[2]
        # test array operation
        # note that the fields vv        vv     vv    are out of order
        crsr.execute("select fldThree,fldFour,fldTwo from tblTemp")
        recs = crsr.fetchall()
        assert recs[1][0] == 103
        assert recs[0][1] == 4
        assert recs[1]['fldFour'] == 104
        assert recs[0,0] == 3
        assert recs[0,'fldTwo'] == 2
        assert recs[1,2] == 102
        for i in range(1):
            for j in range(2):
                assert recs[i][j] == recs[i,j]

    def testFormatParamstyle(self):
        self.helpForceDropOnTblTemp()
        conn=self.getConnection()
        conn.paramstyle = 'format'  #test nonstandard use of paramstyle
        crsr=conn.cursor()
        tabdef= """
            CREATE TABLE tblTemp (
                fldId integer NOT NULL,
                fldData varchar(10))
                """
        crsr.execute(tabdef)

        inputs = [u'one',u'two',u'three']
        fldId=2
        for inParam in inputs:
            fldId+=1
            try:
                crsr.execute("INSERT INTO tblTemp (fldId,fldData) VALUES (%s,%s)", (fldId,inParam))
            except:
                conn.printADOerrors()
                raise
            crsr.execute("SELECT fldData FROM tblTemp WHERE %s=fldID", [fldId])
            rec = crsr.fetchone()
            assert rec[0]==inParam, 'returned value:"%s" != test value:"%s"'%(rec[0],inParam)
            
        # now try an operation with a "%s" as part of a literal
        sel = "insert into tblTemp (fldId,fldData) VALUES (%s,'four%sfive')"
        params = (20,)
        crsr.execute(sel,params)

        #test the .query implementation
        assert '(?,' in crsr.query, 'expected:"%s" in "%s"'%('(?,',crsr.query)
        #test the .command attribute
        assert crsr.command == sel
        #test the .parameters attribute
        assert crsr.parameters == params
        #now make sure the data made it
        crsr.execute("SELECT fldData FROM tblTemp WHERE fldID=20")
        rec = crsr.fetchone()
        assert rec[0]=='four%sfive'

    def testNamedParamstyle(self):
        self.helpForceDropOnTblTemp()
        conn=self.getConnection()
        crsr=conn.cursor()
        crsr.paramstyle = 'named'  #test nonstandard use of paramstyle
        tabdef= """
            CREATE TABLE tblTemp (
                fldId integer NOT NULL,
                fldData varchar(10))
                """
        crsr.execute(tabdef)

        inputs = [u'four',u'five',u'six']
        fldId=3
        for inParam in inputs:
            fldId+=1
            try:
                crsr.execute("INSERT INTO tblTemp (fldId,fldData) VALUES (:Id,:f_Val)", {"f_Val":inParam,'Id':fldId})
            except:
                conn.printADOerrors()
                raise
            crsr.execute("SELECT fldData FROM tblTemp WHERE :Id=fldID", {'Id':fldId})
            rec = crsr.fetchone()
            assert rec[0]==inParam, 'returned value:"%s" != test value:"%s"'%(rec[0],inParam)
        # now a test with a ":" as part of a literal
        crsr.execute("insert into tblTemp (fldId,fldData) VALUES (:xyz,'six:five')",{'xyz':30})
        crsr.execute("SELECT fldData FROM tblTemp WHERE fldID=30")
        rec = crsr.fetchone()
        assert rec[0]=='six:five'
                     
class TestADOwithSQLServer(CommonDBTests):
    def setUp(self):
        self.conn=adodbapi.connect(config.connStrSQLServer)
        self.engine = 'MSSQL'

    def tearDown(self):
        try:
            self.conn.rollback()
        except:
            pass
        try:
            self.conn.close()
        except:
            pass
        self.conn=None
            
    def getConnection(self):
        return self.conn

    def testSQLServerDataTypes(self):
        self.helpTestDataType("decimal(18,2)",'NUMBER',3.45,
                              allowedReturnValues=[u'3.45',u'3,45',decimal.Decimal('3.45')])
        self.helpTestDataType("numeric(18,2)",'NUMBER',3.45,
                              allowedReturnValues=[u'3.45',u'3,45',decimal.Decimal('3.45')])

    def testUserDefinedConversionForExactNumericTypes(self):
        # variantConversions is a dictionary of convertion functions
        # held internally in adodbapi

        oldconverter = adodbapi.variantConversions[ado_consts.adNumeric] #keep old function to restore later
        
        # By default decimal and "numbers" are returned as decimals.
        # Instead, make numbers return as  floats

        adodbapi.variantConversions[ado_consts.adNumeric] = adodbapi.cvtFloat
        self.helpTestDataType("decimal(18,2)",'NUMBER',3.45,compareAlmostEqual=1)
        self.helpTestDataType("numeric(18,2)",'NUMBER',3.45,compareAlmostEqual=1)        
        # now return strings
        adodbapi.variantConversions[ado_consts.adNumeric] = adodbapi.cvtString
        self.helpTestDataType("numeric(18,2)",'NUMBER','3.45')
        # now a completly weird user defined convertion
        adodbapi.variantConversions[ado_consts.adNumeric] = lambda x: u'!!This function returns a funny unicode string %s!!'%x
        self.helpTestDataType("numeric(18,2)",'NUMBER','3.45',
                              allowedReturnValues=[u'!!This function returns a funny unicode string 3.45!!'])

        # now reset the converter to its original function
        adodbapi.variantConversions[ado_consts.adNumeric]=oldconverter #Restore the original convertion function
        self.helpTestDataType("numeric(18,2)",'NUMBER',decimal.Decimal('3.45'))

    def testVariableReturningStoredProcedure(self):
        crsr=self.conn.cursor()
        spdef= """
            CREATE PROCEDURE sp_DeleteMeOnlyForTesting
                @theInput varchar(50),
                @theOtherInput varchar(50),
                @theOutput varchar(100) OUTPUT
            AS
                SET @theOutput=@theInput+@theOtherInput
                    """
        try:
            crsr.execute("DROP PROCEDURE sp_DeleteMeOnlyForTesting")
            self.conn.commit()
        except: #Make sure it is empty
            pass
        crsr.execute(spdef)

        retvalues=crsr.callproc('sp_DeleteMeOnlyForTesting',('Dodsworth','Anne','              '))

        assert retvalues[0]=='Dodsworth', '%s is not "Dodsworth"'%repr(retvalues[0])
        assert retvalues[1]=='Anne','%s is not "Anne"'%repr(retvalues[1])
        assert retvalues[2]=='DodsworthAnne','%s is not "DodsworthAnne"'%repr(retvalues[2])
        self.conn.rollback()
       
    def testMultipleSetReturn(self):
        crsr=self.getCursor()
        self.helpCreateAndPopulateTableTemp(crsr)
        
        spdef= """
            CREATE PROCEDURE sp_DeleteMe_OnlyForTesting
            AS
                SELECT fldData FROM tblTemp ORDER BY fldData ASC
                SELECT fldData From tblTemp where fldData = -9999
                SELECT fldData FROM tblTemp ORDER BY fldData DESC
                    """
        try:
            crsr.execute("DROP PROCEDURE sp_DeleteMe_OnlyForTesting")
            self.conn.commit()
        except: #Make sure it is empty
            pass
        crsr.execute(spdef)

        retvalues=crsr.callproc('sp_DeleteMe_OnlyForTesting')
        row=crsr.fetchone()
        self.assertEquals(row[0], 0) 
        assert crsr.nextset() == True, 'Operation should succede'
        assert not crsr.fetchall(), 'Should be an empty second set'
        assert crsr.nextset() == True, 'third set should be present'
        rowdesc=crsr.fetchall()
        self.assertEquals(rowdesc[0][0],8) 
        assert crsr.nextset() == None,'No more return sets, should return None'

        self.helpRollbackTblTemp()

    def testRollBack(self):
        crsr=self.getCursor()
        self.helpCreateAndPopulateTableTemp(crsr)
        self.conn.commit()

        crsr.execute("INSERT INTO tblTemp (fldData) VALUES(100)")

        selectSql="SELECT fldData FROM tblTemp WHERE fldData=100"
        crsr.execute(selectSql)
        rs=crsr.fetchall()
        assert len(rs)==1
        self.conn.rollback()
        crsr.execute(selectSql)
        assert crsr.fetchone()==None, 'cursor.fetchone should return None if a query retrieves no rows'
        self.helpRollbackTblTemp()
        
  
 
class TestADOwithAccessDB(CommonDBTests):
    def setUp(self):
        self.conn = adodbapi.connect(config.connStrAccess)
        self.engine = 'ACCESS'

    def tearDown(self):
        try:
            self.conn.rollback()
        except:
            pass
        try:
            self.conn.close()
        except:
            pass
        self.conn=None
            
    def getConnection(self):
        return self.conn

    def testOkConnect(self):
        c=adodbapi.connect(config.connStrAccess)
        assert c != None
        c.close()
        
class TestADOwithMySql(CommonDBTests):
    def setUp(self):
        self.conn = adodbapi.connect(config.connStrMySql)
        self.engine = 'MySQL'

    def tearDown(self):
        try:
            self.conn.rollback()
        except:
            pass
        try:
            self.conn.close()
        except:
            pass
        self.conn=None

    def getConnection(self):
        return self.conn

    def testOkConnect(self):
        c=adodbapi.connect(config.connStrMySql)
        assert c != None

class TestADOwithPostgres(CommonDBTests):
    def setUp(self):
        self.conn = adodbapi.connect(config.connStrPostgres)
        self.engine = 'PostgreSQL'

    def tearDown(self):
        try:
            self.conn.rollback()
        except:
            pass
        try:
            self.conn.close()
        except:
            pass
        self.conn=None

    def getConnection(self):
        return self.conn

    def testOkConnect(self):
        c=adodbapi.connect(config.connStrPostgres)
        assert c != None
                
class TimeConverterInterfaceTest(unittest.TestCase):
    def testIDate(self):
        assert self.tc.Date(1990,2,2)

    def testITime(self):
        assert self.tc.Time(13,2,2)

    def testITimestamp(self):
        assert self.tc.Timestamp(1990,2,2,13,2,1)

    def testIDateObjectFromCOMDate(self):
        assert self.tc.DateObjectFromCOMDate(37435.7604282)

    def testICOMDate(self):
        assert hasattr(self.tc,'COMDate')

    def testExactDate(self):
        d=self.tc.Date(1994,11,15)
        comDate=self.tc.COMDate(d)
        correct=34653.0
        assert comDate == correct,comDate
        
    def testExactTimestamp(self):
        d=self.tc.Timestamp(1994,11,15,12,0,0)
        comDate=self.tc.COMDate(d)
        correct=34653.5
        self.assertEquals( comDate ,correct)
        
        d=self.tc.Timestamp(2003,5,6,14,15,17)
        comDate=self.tc.COMDate(d)
        correct=37747.593946759262
        self.assertEquals( comDate ,correct)

    def testIsoFormat(self):
        d=self.tc.Timestamp(1994,11,15,12,3,10)
        iso=self.tc.DateObjectToIsoFormatString(d)
        self.assertEquals(str(iso[:19]) , '1994-11-15 12:03:10')
        
        dt=self.tc.Date(2003,5,2)
        iso=self.tc.DateObjectToIsoFormatString(dt)
        self.assertEquals(str(iso[:10]), '2003-05-02')
        
if config.doMxDateTimeTest:
    import mx.DateTime    
class TestMXDateTimeConverter(TimeConverterInterfaceTest):
    def setUp(self):     
        self.tc=adodbapi.mxDateTimeConverter()
  
    def testCOMDate(self):       
        t=mx.DateTime.DateTime(2002,6,28,18,15,2)       
        cmd=self.tc.COMDate(t)       
        assert cmd == t.COMDate()
    
    def testDateObjectFromCOMDate(self):
        cmd=self.tc.DateObjectFromCOMDate(37435.7604282)
        t=mx.DateTime.DateTime(2002,6,28,18,15,0)
        t2=mx.DateTime.DateTime(2002,6,28,18,15,2)
        assert t2>cmd>t
    
    def testDate(self):
        assert mx.DateTime.Date(1980,11,4)==self.tc.Date(1980,11,4)

    def testTime(self):
        assert mx.DateTime.Time(13,11,4)==self.tc.Time(13,11,4)

    def testTimestamp(self):
        t=mx.DateTime.DateTime(2002,6,28,18,15,1)   
        obj=self.tc.Timestamp(2002,6,28,18,15,1)
        assert t == obj

import time
class TestPythonTimeConverter(TimeConverterInterfaceTest):
    def setUp(self):
        self.tc=adodbapi.pythonTimeConverter()
    
    def testCOMDate(self):
        mk = time.mktime((2002,6,28,18,15,1, 4,31+28+31+30+31+28,-1))
        t=time.localtime(mk)
        # Fri, 28 Jun 2002 18:15:01 +0000
        cmd=self.tc.COMDate(t)
        assert abs(cmd - 37435.7604282) < 1.0/24,"%f more than an hour wrong" % cmd

    def testDateObjectFromCOMDate(self):
        cmd=self.tc.DateObjectFromCOMDate(37435.7604282)
        t1=time.gmtime(time.mktime((2002,6,28,0,14,1, 4,31+28+31+30+31+28,-1)))
        #there are errors in the implementation of gmtime which we ignore
        t2=time.gmtime(time.mktime((2002,6,29,12,14,2, 4,31+28+31+30+31+28,-1)))
        assert t1<cmd<t2, '"%s" should be about 2002-6-28 12:15:01'%repr(cmd)
    
    def testDate(self):
        t1=time.mktime((2002,6,28,18,15,1, 4,31+28+31+30+31+30,0))
        t2=time.mktime((2002,6,30,18,15,1, 4,31+28+31+30+31+28,0))
        obj=self.tc.Date(2002,6,29)
        assert t1< time.mktime(obj)<t2,obj

    def testTime(self):
        self.assertEquals( self.tc.Time(18,15,2),time.gmtime(18*60*60+15*60+2))

    def testTimestamp(self):
        t1=time.localtime(time.mktime((2002,6,28,18,14,1, 4,31+28+31+30+31+28,-1)))
        t2=time.localtime(time.mktime((2002,6,28,18,16,1, 4,31+28+31+30+31+28,-1)))
        obj=self.tc.Timestamp(2002,6,28,18,15,2)
        assert t1< obj <t2,obj

if config.doDateTimeTest:
    import datetime
class TestPythonDateTimeConverter(TimeConverterInterfaceTest):
    def setUp(self):
        self.tc=adodbapi.pythonDateTimeConverter()
    
    def testCOMDate(self):
        t=datetime.datetime( 2002,6,28,18,15,1)
        # Fri, 28 Jun 2002 18:15:01 +0000
        cmd=self.tc.COMDate(t)
        assert abs(cmd - 37435.7604282) < 1.0/24,"more than an hour wrong"
        
    def testDateObjectFromCOMDate(self):
        cmd=self.tc.DateObjectFromCOMDate(37435.7604282)
        t1=datetime.datetime(2002,6,28,18,14,1)
        t2=datetime.datetime(2002,6,28,18,16,1)
        assert t1<cmd<t2,cmd
    
    def testDate(self):
        t1=datetime.date(2002,6,28)
        t2=datetime.date(2002,6,30)
        obj=self.tc.Date(2002,6,29)
        assert t1< obj <t2,obj

    def testTime(self):
        self.assertEquals( self.tc.Time(18,15,2).isoformat()[:8],'18:15:02')

    def testTimestamp(self):
        t1=datetime.datetime(2002,6,28,18,14,1)
        t2=datetime.datetime(2002,6,28,18,16,1)
        obj=self.tc.Timestamp(2002,6,28,18,15,2)
        assert t1< obj <t2,obj

        
suites=[]
if config.doMxDateTimeTest:
    suites.append( unittest.makeSuite(TestMXDateTimeConverter,'test'))
if config.doDateTimeTest:
    suites.append( unittest.makeSuite(TestPythonDateTimeConverter,'test'))
suites.append( unittest.makeSuite(TestPythonTimeConverter,'test'))

if config.doAccessTest:
    suites.append( unittest.makeSuite(TestADOwithAccessDB,'test'))
if config.doSqlServerTest:    
    suites.append( unittest.makeSuite(TestADOwithSQLServer,'test'))
if config.doMySqlTest:
    suites.append( unittest.makeSuite(TestADOwithMySql,'test'))
if config.doPostgresTest:
    suites.append( unittest.makeSuite(TestADOwithPostgres,'test'))
    
suite=unittest.TestSuite(suites)
if __name__ == '__main__':       
    defaultDateConverter=adodbapi.dateconverter
    print __doc__
    print "Default Date Converter is %s" %(defaultDateConverter,)
    unittest.TextTestRunner().run(suite)
    if config.iterateOverTimeTests:
        for test,dateconverter in (
                    (1,adodbapi.pythonTimeConverter),
                    (config.doMxDateTimeTest,adodbapi.mxDateTimeConverter),
                    (config.doDateTimeTest,adodbapi.pythonDateTimeConverter)
                                    ):
            if test and not isinstance(defaultDateConverter,dateconverter):
                adodbapi.dateconverter=dateconverter()
                print "Changed dateconverter to "
                print adodbapi.dateconverter
                unittest.TextTestRunner().run(suite)
