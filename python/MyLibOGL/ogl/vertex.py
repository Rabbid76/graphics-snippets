import sys
import math
#import array

# Numpy improt [http://www.numpy.org/]
import numpy

# PyOpenGL import [http://pyopengl.sourceforge.net/]
from OpenGL.GL import *

# TODO numpy -> array [https://docs.python.org/3/library/array.html]
# TODO use double array


# vertex array object
class VAObject:
    # dataArrays: e.g. [ (3, [Vx0, Vy0, Vz0, Vx1, Vy1, Vz1, .... ]), (3, [ Nx0, Ny0, Nz0, .... ]), (3, [ Cr0, Cg0, Cb0, .... ]), (2, [ Tu0, Tv0, ..... ]) ]
    def __init__( self, dataArrays, indices = [], type = GL_TRIANGLES, patch_vertices = 3 ):
        self.__obj = glGenVertexArrays( 1 )
        self.__noOfIndices = len( indices )
        self.__indexArr = numpy.array( indices, dtype=numpy.uint32 )
        self.__type = type
        self.__patch_vertices = patch_vertices
        self.__vertexSize = []
        self.__dataLength = []
        self.__noOfBuffers = len( dataArrays )
        self.__buffers = glGenBuffers( self.__noOfBuffers )
        glBindVertexArray( self.__obj )
        for i_buffer in range( 0, self.__noOfBuffers ):
            vertexSize, dataArr = dataArrays[i_buffer]
            self.__vertexSize.append( vertexSize )
            self.__dataLength.append( len( dataArr ) )
            glBindBuffer( GL_ARRAY_BUFFER, self.__buffers if self.__noOfBuffers == 1 else self.__buffers[i_buffer] )
            glBufferData( GL_ARRAY_BUFFER, numpy.array( dataArr, dtype=numpy.float32 ), GL_STATIC_DRAW )
            glEnableVertexAttribArray( i_buffer )
            glVertexAttribPointer( i_buffer, self.__vertexSize[i_buffer], GL_FLOAT, GL_FALSE, 0, None )
        self.__iBuffer = glGenBuffers( 1 )
        glBindBuffer( GL_ARRAY_BUFFER, 0 )
        if self.__noOfIndices > 0:
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, self.__iBuffer )
            glBufferData( GL_ELEMENT_ARRAY_BUFFER, self.__indexArr, GL_STATIC_DRAW )
        glBindVertexArray( 0 )
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 )
    def DrawArray(self):
        glBindVertexArray( self.__obj )
        if self.__type == GL_PATCHES:
            glPatchParameteri( GL_PATCH_VERTICES, self.__patch_vertices )
        glDrawArrays( self.__type, 0, self.__dataLength[0] )
        glBindVertexArray( 0 )
    def Draw(self):
        if self.__noOfIndices == 0:
            self.DrawArray()
            return
        glBindVertexArray( self.__obj )
        #for i_buffer in range( 0, self.__noOfBuffers ):
        #    glBindBuffer( GL_ARRAY_BUFFER, self.__buffers if self.__noOfBuffers == 1 else self.__buffers[i_buffer] )
        #    glEnableVertexAttribArray( i_buffer )
        #    glVertexAttribPointer( i_buffer, self.__vertexSize[i_buffer], GL_FLOAT, GL_FALSE, 0, None )
        #glDrawElements( self.__type, self.__noOfIndices, GL_UNSIGNED_INT, self.__indexArr )
        if self.__type == GL_PATCHES:
            glPatchParameteri( GL_PATCH_VERTICES, self.__patch_vertices )
        glDrawElements( self.__type, self.__noOfIndices, GL_UNSIGNED_INT, None )
        glBindVertexArray( 0 )


# vertex array object specification (stride or tight)
class MeshBuffer:
    
    # ctor
    def __init__(self, usage = GL_STATIC_DRAW):
        self.__usage = usage
        self.__vao = 0
        self.__vbo = {}
        self.__ibo = { 'ibo': 0, 'size': 0 }

    # dtor
    def __del__(self):
        vbos = []
        for i_buffer in self.__vbo:
            vbos.append( self.__vbo[i_buffer]['vbo'] ) 
        glDeleteBuffers( len(vbos), vbos )
        if self.__ibo['ibo'] != 0:
            glDeleteBuffers( 1, self.__ibo['ibo'] )
        if self.__vao != 0:
            glDeleteVertexArrays( 1, self.__vao )

    def __BindVA(self):
        if self.__vao == 0:
            self.__vao = glGenVertexArrays( 1 )
        glBindVertexArray( self.__vao )

    # define a list of vertex buffers
    def DefineVA(self, buffers):
        self.__BindVA()
        # for all buffers
        for i_buffer in range(len(buffers)):
            self.DefineVB(i_buffer, buffers[i_buffer])
        glBindVertexArray( 0 )

    # define a singel vertex buffer
    def DefineVB(self, i_buffer, buffer):
        if not (i_buffer in self.__vbo):
            self.__vbo[i_buffer] = { 'vbo': 0, 'stride': 0, 'size': 0, 'attribs': [] } 
        # for all attributs in buffer
        attribs, stride, data = buffer
        size = len(data)
        if self.__vbo[i_buffer]['vbo'] == 0:
            self.__vbo[i_buffer]['vbo'] = glGenBuffers( 1 )
        glBindBuffer( GL_ARRAY_BUFFER, self.__vbo[i_buffer]['vbo'] )
        arrdata = numpy.array( data, dtype=numpy.float32 )    
        if self.__vbo[i_buffer]['size'] < size:
            glBufferData( GL_ARRAY_BUFFER, arrdata, self.__usage )
            self.__vbo[i_buffer]['size'] = size
        else:
            glBufferSubData( GL_ARRAY_BUFFER, 0, arrdata, self.__usage )   
        for i_attrib in range(len(attribs)):
            attrib_index, attrib_size, attrib_offset = attribs[i_attrib]
            self.__vbo[i_buffer]['attribs'].append( (attrib_index, attrib_size, attrib_offset) )
            glEnableVertexAttribArray( attrib_index )
            if stride==0:
                glVertexAttribPointer( attrib_index, attrib_size, GL_FLOAT, GL_FALSE, stride, None )
            else:
                glVertexAttribPointer( attrib_index, attrib_size, GL_FLOAT, GL_FALSE, stride, attrib_offset )
        glBindBuffer( GL_ARRAY_BUFFER, 0 ) 

    def DefineIB(self, indices):
        self.__BindVA()
        if self.__ibo['ibo'] == 0:
            self.__ibo['ibo'] = glGenBuffers( 1 )
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, self.__ibo['ibo'] )
        size = len(indices)
        indexdata = numpy.array( indices, dtype=numpy.uint32 )
        if self.__ibo['size'] < size:
            glBufferData( GL_ELEMENT_ARRAY_BUFFER, indexdata, self.__usage )
            self.__ibo['size'] = size
        else:   
            glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, 0, indexdata, self.__usage ) 
        glBindVertexArray( 0 )
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ) # has to be unbind after vertex array object was unbound!

    def DrawIB(self, type):
        glBindVertexArray( self.__vao )
        glDrawElements( type, self.__ibo['size'], GL_UNSIGNED_INT, None )
        glBindVertexArray( 0 )


CLIENT_VERTEX    = -1 # GL_VERTEX_ARRAY
CLIENT_NORMAL    = -2 # GL_NORMAL_ARRAY
CLIENT_TEXTURE   = -3 # GL_TEXTURE_COORD_ARRAY
CLIENT_COLOR     = -4 # GL_COLOR_ARRAY
CLIENT_COLOR_2   = -5 # GL_SECONDARY_COLOR_ARRAY
CLIENT_INDEX     = -6 # GL_INDEX_ARRAY
CLIENT_EDGE_FLAG = -7 # GL_EDGE_FLAG_ARRAY
CLIENT_FOG_COORD = -8 # GL_FOG_COORD_ARRAY   


# class which creates and manages vertex array objects with the following description
#
# <index buffer>              : index of the index buffer; 0 is default; < -1 no index buffer is reqired
# <no of buffers>             : the number of the require vertex buffer objects - followed by the list of vbo specifications
# {
#     <stride>                : stride from one vertex attribute set to the next, in float (4 byte) units) 
#     <no of attributes>      : number of the generic vertex attributes in the buffer - followed by the list of attribute specifications
#     {
#         <attribute index>   : vertex attribute index or client state
#         <attribute size>    : number of elemts of the vertex attribute  
#         <attribute offset>  : offset of the vertex attributes from the begin of the attributes set
#     }
# }
#
#
# e.g. Strided record sets:  
#      Vx0, Vy0, Vz0, Nx0, Ny0, Nz0, Tu0, Tv0, Vx1, Vy1, Vz1, Nx1, Ny1, Nz1, Tu1, Tv1, ....
#      
#      [0, 1, 8, 3, -1, 3, 0, -2, 3, 3, -3, 2, 6]
#
#
# e.g. Tightly packed vertex attributes:
#      Vx0, Vy0, Vz0, Vx1, Vy1, Vz1, .... 
#      Nx0, Ny0, Nz0, Nx1, Ny1, Nz1, ....
#      Tu0, Tv0, Tu1, Tv1 ....
#
#      [0, 3, 0, 1, -1, 3, 0, 1, 0, -2, 3, 0, 1, 0, -3, 2, 0]
#
class DrawBuffer:

    # ctor
    def __init__(self, usage = GL_STREAM_DRAW):
        self.__usage       = usage # usage type of buffer objects GL_STATIC_DRAW, GL_DYNAMIC_DRAW, GL_STREAM_DRAW
        self.__currVAO     = 0     # current selected vertex array object <GPU name>
        self.__currNoElems = 0     # number of elements in the curently selected vertex array object
        self.__vaos = {}           # map destcription -> (vertex array object <GPU name>, description)
        self.__ibos = {}           # map index -> ( element array buffer <GPU name>, size <count> of element array buffer )
        self.__vbos = []           # list of array buffers ( array buffer <GPU name>, size <count> of array buffer )


    # dtor
    def __del__(self):
        
        # delete array buffers
        vbos = []
        for vbo in self.__vbos: vbos.append( vbo[0] ) 
        if len(vbos) > 0:       glDeleteBuffers( len(vbos), vbos )
        
        # delete element array buffers
        ibos = []
        for i_ibo in self.__ibos: ibos.append( self.__ibos[i_ibo][0] ) 
        if len(ibos) > 0:       glDeleteBuffers( len(ibos), ibos )
        
        # delete vertex array objects
        vaos = []
        for i_vao in self.__vaos: vaos.append( self.__vaos[i_vao][0] ) 
        if len(vaos) > 0:       glDeleteVertexArrays( len(vaos), vaos )


    # unbind any vertex array object
    def UnbindVAO(self):
        glBindVertexArray( 0 )


    # bind the currently selected vertex array object
    def BindVAO(self):
        if self.__currVAO == 0:
            print( "No vertex array object is selected" )
            sys.exit()
        glBindVertexArray( self.__currVAO )


    # define and enable an array of generic vertex attribute
    # or define and enable a clinet array
    def DefineAndEnableAttribute(self, attr_id, attr_size, attr_offs, stride):
    
        # define and enable an array of generic vertex attribute
        if attr_id >= 0:
            glVertexAttribPointer( attr_id, attr_size, GL_FLOAT, GL_FALSE, stride, None if stride == 0 else attr_offs )
            glEnableVertexAttribArray( attr_id )
        
        # define an array of generic vertex attribute data
        elif attr_id == CLIENT_VERTEX:
            glVertexPointer( attr_size, GL_FLOAT, stride, None if stride == 0 else attr_offs ) 
            glEnableClientState( GL_VERTEX_ARRAY )

        # define an array of normals
        elif attr_id == CLIENT_GL_NORMAL_ARRAYVERTEX:
            glNormalPointer( GL_FLOAT, stride, None if stride == 0 else attr_offs ) 
            glEnableClientState( GL_NORMAL_ARRAY ) 

        # define an array of texture coordinates
        elif attr_id == CLIENT_VERTEX:
            glTexCoordPointer( attr_size, GL_FLOAT, stride, None if stride == 0 else attr_offs ) 
            glEnableClientState( GL_TEXTURE_COORD_ARRAY ) 

        # define an array of colors
        elif attr_id == CLIENT_COLOR:
            glColorPointer( attr_size, GL_FLOAT, stride, None if stride == 0 else attr_offs ) 
            glEnableClientState( GL_COLOR_ARRAY )

        # define an array of secondary colors
        elif attr_id == CLIENT_COLOR_2:
            glSecondaryColorPointer( attr_size, GL_FLOAT, stride, None if stride == 0 else attr_offs ) 
            glEnableClientState( GL_SECONDARY_COLOR_ARRAY ) 

        # define an array of color indexes
        elif attr_id == CLIENT_INDEX:
            glIndexPointer( attr_size, stride, None if stride == 0 else attr_offs ) 
            glEnableClientState( GL_INDEX_ARRAY ) 

        # define an array of edge flags
        elif attr_id == CLIENT_EDGE_FLAG:
            glEdgeFlagPointer ( stride, None if stride == 0 else attr_offs ) 
            glEnableClientState( GL_EDGE_FLAG_ARRAY )

        # define an array of fog coordinates
        elif attr_id == CLIENT_FOG_COORD:
            glFogCoordPointer ( attr_size, stride, None if stride == 0 else attr_offs ) 
            glEnableClientState( GL_FOG_COORD_ARRAY ) 

        else:
            print( "Illegal vertex array index or client state" )
            sys.exit() 


    # set data to buffer
    def UpdateBuffer(self, type, i_bo, bos, data ):

        bo, size = bos[i_bo]
        new_data = data
        new_size = len(new_data) 

        glBindBuffer( type, bo )
        if new_size > size: # the buffer has to be enlarged - slow
            glBufferData( type, data, self.__usage )
            bos[i_bo] = bo, new_size
        else:               # the buffer is lared enough - quick
            glBufferSubData( type, 0, data, self.__usage ) 

        return bos[i_bo][1]


    # If a vertex array object with the description exists, then it is becomes the current vertex array object.  
    # If no vertex array object exists, which follows the description, then a new vertex array object is created and the new object is made the current object.
    # Note, the vertex array object is not bound, it is unbound
    def ProvideVAO(self, description):
        
        # Create description key array
        key = numpy.array( description, dtype=numpy.int8 )
        hashcode = 5381
        for c in key:
            hashcode = hashcode * 33 + hashcode + c
        
        # Check if a proper vertex array object already exists
        if hashcode in self.__vaos:
            self.__currVAO = self.__vaos[hashcode][0]
            return hashcode  

        # Check if the required buffers are exists and create them if they are not created yet.
        i_ibo = key[0]
        no_of_vbo = key[1]
        if i_ibo >= 0 and (i_ibo not in self.__ibos): self.__ibos[i_ibo] = (glGenBuffers( 1 ), 0)
        if no_of_vbo > len(self.__vbos):              vbos = glGenBuffers( no_of_vbo )
        for vbo in vbos:                              self.__vbos.append((vbo, 0) )

        # Create vertex array object
        self.__vaos[hashcode] = (glGenVertexArrays( 1 ), key) 
        self.__currVAO = self.__vaos[hashcode][0]
        self.BindVAO() 
          
        # Create a new vertex array opject according to the description
        i_key = 2
        for i_vbo in range(no_of_vbo):
            stride, no_of_attr = key[i_key], key[i_key+1]
            i_key = i_key + 2
            glBindBuffer( GL_ARRAY_BUFFER, self.__vbos[i_vbo][0] )
            for i_attr in range(no_of_attr):
                attr_id, attr_size, attr_offs = key[i_key], key[i_key+1],  key[i_key+2]
                i_key = i_key + 3
                self.DefineAndEnableAttribute( attr_id, attr_size, attr_offs, stride )
        glBindBuffer( GL_ARRAY_BUFFER, 0 )

        # Associate the element array buffer (index buffer) to the vertex array object
        if i_ibo >= 0:
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, self.__ibos[i_ibo][0] )    

        # Unbind the vertex array object
        self.UnbindVAO()    

        # Unbinde the element array buffer
        # This has to be done after the vertex array object is unbound, otherwise the association to the vertex array object would be lost.
        if i_ibo >= 0:
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ) 

        return hashcode


    # define a vertex array object
    def DefineVAO(self, description, buffers, indices):

        # Find or create the vertex array object
        hashcode = self.ProvideVAO( description )
        
        # update vertex attribut data
        if isinstance(buffers, (list, tuple)): # check if list of buffes
            for i_vbo in range(len(buffers)):
                arrdata = numpy.array( buffers[i_vbo], dtype=numpy.float32 )  
                self.UpdateBuffer( GL_ARRAY_BUFFER, i_vbo, self.__vbos, arrdata )
        else: # singe buffer
            arrdata = numpy.array( buffers, dtype=numpy.float32 )  
            self.UpdateBuffer( GL_ARRAY_BUFFER, 0, self.__vbos, arrdata )
        glBindBuffer( GL_ARRAY_BUFFER, 0 ) 

        # update indices data
        i_ibo = description[0]
        if i_ibo >= 0:
            indexdata = numpy.array( indices, dtype=numpy.uint32 ) 
            self.__currNoElems  = self.UpdateBuffer( GL_ELEMENT_ARRAY_BUFFER, i_ibo, self.__ibos, indexdata ) 
        else:
            self.__currNoElems = 0
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 )  

        # bind vertex attribute object
        self.BindVAO()
        return hashcode

    # draw all elements of the current vertex array object
    def DrawAllElements(self, type):
        self.BindVAO()
        glDrawElements( type, self.__currNoElems , GL_UNSIGNED_INT, None )
        self.UnbindVAO()


