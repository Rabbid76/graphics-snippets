glMatrixArrayType=typeof Float32Array!="undefined"?Float32Array:typeof WebGLFloatArray!="undefined"?WebGLFloatArray:Array;


//! /class vec3
//!
//!
var vec3={};

vec3.create = function(a){
  var b=new glMatrixArrayType(3);
  if(a){
    b[0]=a[0];b[1]=a[1];
    b[2]=a[2]
  }
  return b
};

vec3.set = function(a,b){
  b[0]=a[0];b[1]=a[1];b[2]=a[2];
  return b
};

vec3.add=function(a,b,c){
  if(!c||a==c){
    a[0]+=b[0];a[1]+=b[1];a[2]+=b[2];
    return a
  }
  c[0]=a[0]+b[0];c[1]=a[1]+b[1];c[2]=a[2]+b[2];
  return c
};

vec3.subtract=function(a,b,c){
  if(!c||a==c){
    a[0]-=b[0];a[1]-=b[1];a[2]-=b[2];
    return a
  }
  c[0]=a[0]-b[0];c[1]=a[1]-b[1];c[2]=a[2]-b[2];
  return c
};

vec3.negate=function(a,b){
  b||(b=a);b[0]=-a[0];b[1]=-a[1];b[2]=-a[2];
  return b
};

vec3.scale=function(a,b,c){
  if(!c||a==c){
    a[0]*=b;a[1]*=b;a[2]*=b;
    return a
  }
  c[0]=a[0]*b;c[1]=a[1]*b;c[2]=a[2]*b;
  return c
};

vec3.normalize=function(a,b){
  b||(b=a);
  var c=a[0],d=a[1],e=a[2],g=Math.sqrt(c*c+d*d+e*e);
  if(g){
    if(g==1){b[0]=c;b[1]=d;b[2]=e;
      return b
    }
  }else{
    b[0]=0;b[1]=0;b[2]=0;
    return b
  }
  g=1/g;b[0]=c*g;b[1]=d*g;b[2]=e*g;
  return b
};

vec3.cross=function(a,b,c){
  c||(c=a);var d=a[0],e=a[1];a=a[2];
  var g=b[0],f=b[1];b=b[2];c[0]=e*b-a*f;c[1]=a*g-d*b;c[2]=d*f-e*g;
  return c
};

vec3.length=function(a){
  var b=a[0],c=a[1];a=a[2];
  return Math.sqrt(b*b+c*c+a*a)
};

vec3.dot=function(a,b){
    return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]
};

vec3.direction=function(a,b,c){
  c||(c=a);
  var d=a[0]-b[0],e=a[1]-b[1];a=a[2]-b[2];b=Math.sqrt(d*d+e*e+a*a);
  if(!b){
    c[0]=0;c[1]=0;c[2]=0;
    return c
  }
  b=1/b;c[0]=d*b;c[1]=e*b;c[2]=a*b;
  return c
};

vec3.lerp=function(a,b,c,d){
  d||(d=a);d[0]=a[0]+c*(b[0]-a[0]);d[1]=a[1]+c*(b[1]-a[1]);d[2]=a[2]+c*(b[2]-a[2]);
  return d
};

vec3.str=function(a){
  return"["+a[0]+", "+a[1]+", "+a[2]+"]"
};


//! /class mat3
//!
//!
var mat3={};

mat3.create=function(a){
  var b=new glMatrixArrayType(9);
  if(a){
    b[0]=a[0];b[1]=a[1];b[2]=a[2];b[3]=a[3];b[4]=a[4];b[5]=a[5];b[6]=a[6];b[7]=a[7];b[8]=a[8];b[9]=a[9]
  }
  return b
};

mat3.set=function(a,b){
  b[0]=a[0];b[1]=a[1];b[2]=a[2];b[3]=a[3];b[4]=a[4];b[5]=a[5];
  b[6]=a[6];b[7]=a[7];b[8]=a[8];
  return b
};

mat3.identity=function(a){
  a[0]=1;a[1]=0;a[2]=0;a[3]=0;a[4]=1;a[5]=0;a[6]=0;a[7]=0;a[8]=1;
  return a
};

mat3.transpose=function(a,b){
  if(!b||a==b){
    var c=a[1],d=a[2],e=a[5];a[1]=a[3];a[2]=a[6];a[3]=c;a[5]=a[7];a[6]=d;a[7]=e;
    return a
  }
  b[0]=a[0];b[1]=a[3];b[2]=a[6];b[3]=a[1];b[4]=a[4];b[5]=a[7];b[6]=a[2];b[7]=a[5];b[8]=a[8];
  return b
};

mat3.toMat4=function(a,b){
  b||(b=mat4.create());b[0]=a[0];b[1]=a[1];b[2]=a[2];b[3]=0;b[4]=a[3];b[5]=a[4];b[6]=a[5];b[7]=0;b[8]=a[6];b[9]=a[7];b[10]=a[8];b[11]=0;b[12]=0;b[13]=0;b[14]=0;b[15]=1;
  return b
};

mat3.str=function(a){
  return"["+a[0]+", "+a[1]+", "+a[2]+", "+a[3]+", "+a[4]+", "+a[5]+", "+a[6]+", "+a[7]+", "+a[8]+"]"
};


//! /class mat4
//!
//!
var mat4={};

mat4.create=function(a){
  var b=new glMatrixArrayType(16);
  if(a){
    b[0]=a[0];b[1]=a[1];b[2]=a[2];b[3]=a[3];b[4]=a[4];b[5]=a[5];b[6]=a[6];b[7]=a[7];b[8]=a[8];b[9]=a[9];b[10]=a[10];b[11]=a[11];b[12]=a[12];b[13]=a[13];b[14]=a[14];b[15]=a[15]
  }
  return b
};

mat4.set=function(a,b){
  b[0]=a[0];b[1]=a[1];b[2]=a[2];b[3]=a[3];b[4]=a[4];b[5]=a[5];b[6]=a[6];b[7]=a[7];b[8]=a[8];b[9]=a[9];b[10]=a[10];b[11]=a[11];b[12]=a[12];b[13]=a[13];b[14]=a[14];b[15]=a[15];
  return b
};

mat4.identity=function(a){
  if(!a) a=new glMatrixArrayType(16);
  a[0]=1;a[1]=0;a[2]=0;a[3]=0;a[4]=0;a[5]=1;a[6]=0;a[7]=0;a[8]=0;a[9]=0;a[10]=1;a[11]=0;a[12]=0;a[13]=0;a[14]=0;a[15]=1;
  return a
};

function makeRotationFromQuaternion( te, q ) {
    if(!te) a=new glMatrixArrayType(16);
		var x = q[0], y = q[1], z = q[2], w = q[3];
		var x2 = x + x, y2 = y + y, z2 = z + z;
		var xx = x * x2, xy = x * y2, xz = x * z2;
		var yy = y * y2, yz = y * z2, zz = z * z2;
		var wx = w * x2, wy = w * y2, wz = w * z2;
		te[ 0 ] = 1 - ( yy + zz );
		te[ 4 ] = xy - wz;
		te[ 8 ] = xz + wy;
		te[ 1 ] = xy + wz;
		te[ 5 ] = 1 - ( xx + zz );
		te[ 9 ] = yz - wx;
		te[ 2 ] = xz - wy;
		te[ 6 ] = yz + wx;
		te[ 10 ] = 1 - ( xx + yy );
		te[ 3 ] = 0;
		te[ 7 ] = 0;
		te[ 11 ] = 0;
		te[ 12 ] = 0;
		te[ 13 ] = 0;
		te[ 14 ] = 0;
		te[ 15 ] = 1;
    return te;
	}

mat4.transpose=function(a,b){
  if(!b||a==b){
    var c=a[1],d=a[2],e=a[3],g=a[6],f=a[7],h=a[11];a[1]=a[4];a[2]=a[8];a[3]=a[12];a[4]=c;a[6]=a[9];a[7]=a[13];a[8]=d;a[9]=g;a[11]=a[14];a[12]=e;a[13]=f;a[14]=h;
    return a
  }
  b[0]=a[0];b[1]=a[4];b[2]=a[8];b[3]=a[12];b[4]=a[1];b[5]=a[5];b[6]=a[9];b[7]=a[13];b[8]=a[2];b[9]=a[6];b[10]=a[10];b[11]=a[14];b[12]=a[3];b[13]=a[7];b[14]=a[11];b[15]=a[15];
  return b
};

mat4.determinant=function(a){
  var b=a[0],c=a[1],d=a[2],e=a[3],g=a[4],f=a[5],h=a[6],i=a[7],j=a[8],k=a[9],l=a[10],o=a[11],m=a[12],n=a[13],p=a[14];a=a[15];
  return m*k*h*e-j*n*h*e-m*f*l*e+g*n*l*e+j*f*p*e-g*k*p*e-m*k*d*i+j*n*d*i+m*c*l*i-b*n*l*i-j*c*p*i+b*k*p*i+m*f*d*o-g*n*d*o-m*c*h*o+b*n*h*o+g*c*p*o-b*f*p*o-j*f*d*a+g*k*d*a+j*c*h*a-b*k*h*a-g*c*l*a+b*f*l*a
};

mat4.inverse=function(a,b){
  b||(b=a);
  var c=a[0],d=a[1],e=a[2],g=a[3],f=a[4],h=a[5],i=a[6],j=a[7],k=a[8],l=a[9],o=a[10],m=a[11],n=a[12],p=a[13],r=a[14],s=a[15];
  var A=c*h-d*f,B=c*i-e*f,t=c*j-g*f,u=d*i-e*h,v=d*j-g*h,w=e*j-g*i,x=k*p-l*n,y=k*r-o*n,z=k*s-m*n;
  var C=l*r-o*p,D=l*s-m*p,E=o*s-m*r,q=1/(A*E-B*D+t*C+u*z-v*y+w*x);
  b[0]=(h*E-i*D+j*C)*q;b[1]=(-d*E+e*D-g*C)*q;b[2]=(p*w-r*v+s*u)*q;b[3]=(-l*w+o*v-m*u)*q;
  b[4]=(-f*E+i*z-j*y)*q;b[5]=(c*E-e*z+g*y)*q;b[6]=(-n*w+r*t-s*B)*q;b[7]=(k*w-o*t+m*B)*q;
  b[8]=(f*D-h*z+j*x)*q;b[9]=(-c*D+d*z-g*x)*q;b[10]=(n*v-p*t+s*A)*q;b[11]=(-k*v+l*t-m*A)*q;
  b[12]=(-f*C+h*y-i*x)*q;b[13]=(c*C-d*y+e*x)*q;b[14]=(-n*u+p*B-r*A)*q;b[15]=(k*u-l*B+o*A)*q;
  return b
};

mat4.toRotationMat=function(a,b){
  b||(b=mat4.create());
  b[0]=a[0];b[1]=a[1];b[2]=a[2];b[3]=a[3];b[4]=a[4];b[5]=a[5];b[6]=a[6];b[7]=a[7];b[8]=a[8];b[9]=a[9];b[10]=a[10];b[11]=a[11];b[12]=0;b[13]=0;b[14]=0;b[15]=1;
  return b
};

mat4.toMat3=function(a,b){
  b||(b=mat3.create());b[0]=a[0];b[1]=a[1];b[2]=a[2];b[3]=a[4];b[4]=a[5];b[5]=a[6];b[6]=a[8];b[7]=a[9];b[8]=a[10];
  return b
};

mat4.toInverseMat3=function(a,b){
  var c=a[0],d=a[1],e=a[2],g=a[4],f=a[5],h=a[6],i=a[8],j=a[9],k=a[10],l=k*f-h*j,o=-k*g+h*i,m=j*g-f*i,n=c*l+d*o+e*m;
  if(!n)
  return null;
  n=1/n;
  b||(b=mat3.create());
  b[0]=l*n;b[1]=(-k*d+e*j)*n;b[2]=(h*d-e*f)*n;b[3]=o*n;b[4]=(k*c-e*i)*n;b[5]=(-h*c+e*g)*n;b[6]=m*n;b[7]=(-j*c+d*i)*n;b[8]=(f*c-d*g)*n;
  return b
};

mat4.multiply = function (a,b,c) {
  c||(c=a);
  var d=a[0],e=a[1],g=a[2],f=a[3],h=a[4],i=a[5],j=a[6],k=a[7],l=a[8],o=a[9],m=a[10],n=a[11],p=a[12],r=a[13],s=a[14];a=a[15];
  var A=b[0],B=b[1],t=b[2],u=b[3],v=b[4],w=b[5],x=b[6],y=b[7],z=b[8],C=b[9],D=b[10],E=b[11],q=b[12],F=b[13],G=b[14];b=b[15];
  c[0]=A*d+B*h+t*l+u*p;
  c[1]=A*e+B*i+t*o+u*r;
  c[2]=A*g+B*j+t*m+u*s;
  c[3]=A*f+B*k+t*n+u*a;
  c[4]=v*d+w*h+x*l+y*p;
  c[5]=v*e+w*i+x*o+y*r;
  c[6]=v*g+w*j+x*m+y*s;
  c[7]=v*f+w*k+x*n+y*a;
  c[8]=z*d+C*h+D*l+E*p;
  c[9]=z*e+C*i+D*o+E*r;
  c[10]=z*g+C*j+D*m+E*s;
  c[11]=z*f+C*k+D*n+E*a;
  c[12]=q*d+F*h+G*l+b*p;
  c[13]=q*e+F*i+G*o+b*r;
  c[14]=q*g+F*j+G*m+b*s;
  c[15]=q*f+F*k+G*n+b*a;
  return c
};

mat4.multiplyVec3=function(a,b,c){
  c||(c=b);
  var d=b[0],e=b[1];b=b[2];c[0]=a[0]*d+a[4]*e+a[8]*b+a[12];c[1]=a[1]*d+a[5]*e+a[9]*b+a[13];c[2]=a[2]*d+a[6]*e+a[10]*b+a[14];
  return c
};

mat4.multiplyVec4=function(a,b,c){
  c||(c=b);
  var d=b[0],e=b[1],g=b[2];b=b[3];c[0]=a[0]*d+a[4]*e+a[8]*g+a[12]*b;c[1]=a[1]*d+a[5]*e+a[9]*g+a[13]*b;c[2]=a[2]*d+a[6]*e+a[10]*g+a[14]*b;c[3]=a[3]*d+a[7]*e+a[11]*g+a[15]*b;
  return c
};

mat4.translate=function(a,b,c){
  var d=b[0],e=b[1];b=b[2];
  if(!c||a==c){
    a[12]=a[0]*d+a[4]*e+a[8]*b+a[12];a[13]=a[1]*d+a[5]*e+a[9]*b+a[13];a[14]=a[2]*d+a[6]*e+a[10]*b+a[14];a[15]=a[3]*d+a[7]*e+a[11]*b+a[15];
    return a
  }
  var g=a[0],f=a[1],h=a[2],i=a[3],j=a[4],k=a[5],l=a[6],o=a[7],m=a[8],n=a[9],p=a[10],r=a[11];c[0]=g;c[1]=f;c[2]=h;c[3]=i;c[4]=j;c[5]=k;c[6]=l;c[7]=o;c[8]=m;c[9]=n;c[10]=p;c[11]=r;c[12]=g*d+j*e+m*b+a[12];c[13]=f*d+k*e+n*b+a[13];c[14]=h*d+l*e+p*b+a[14];c[15]=i*d+o*e+r*b+a[15];
  return c
};

mat4.scale=function(a,b,c){
  var d=b[0],e=b[1];b=b[2];
  if(!c||a==c){
    a[0]*=d;a[1]*=d;a[2]*=d;a[3]*=d;a[4]*=e;a[5]*=e;a[6]*=e;a[7]*=e;a[8]*=b;a[9]*=b;a[10]*=b;a[11]*=b;
    return a
  }
  c[0]=a[0]*d;c[1]=a[1]*d;c[2]=a[2]*d;c[3]=a[3]*d;c[4]=a[4]*e;c[5]=a[5]*e;c[6]=a[6]*e;c[7]=a[7]*e;c[8]=a[8]*b;c[9]=a[9]*b;c[10]=a[10]*b;c[11]=a[11]*b;c[12]=a[12];c[13]=a[13];c[14]=a[14];c[15]=a[15];
  return c
};

mat4.rotate=function(a,b,c,d){
  var e=c[0],g=c[1];c=c[2];
  var f=Math.sqrt(e*e+g*g+c*c);
  if(!f)return null;
  if(f!=1){
    f=1/f;e*=f;g*=f;c*=f
  }
  var h=Math.sin(b),i=Math.cos(b),j=1-i;b=a[0];f=a[1];
  var k=a[2],l=a[3],o=a[4],m=a[5],n=a[6],p=a[7],r=a[8],s=a[9],A=a[10],B=a[11],t=e*e*j+i,u=g*e*j+c*h,v=c*e*j-g*h,w=e*g*j-c*h,x=g*g*j+i,y=c*g*j+e*h,z=e*c*j+g*h;e=g*c*j-e*h;g=c*c*j+i;
  if(d){
    if(a!=d){
      d[12]=a[12];d[13]=a[13];d[14]=a[14];d[15]=a[15]
    }
  }
  else
    d=a;
  d[0]=b*t+o*u+r*v;d[1]=f*t+m*u+s*v;d[2]=k*t+n*u+A*v;d[3]=l*t+p*u+B*v;d[4]=b*w+o*x+r*y;d[5]=f*w+m*x+s*y;d[6]=k*w+n*x+A*y;d[7]=l*w+p*x+B*y;d[8]=b*z+o*e+r*g;d[9]=f*z+m*e+s*g;d[10]=k*z+n*e+A*g;d[11]=l*z+p*e+B*g;
  return d
};

mat4.rotateX=function(a,b,c){
  var d=Math.sin(b);b=Math.cos(b);
  var e=a[4],g=a[5],f=a[6],h=a[7],i=a[8],j=a[9],k=a[10],l=a[11];
  if(c){
    if(a!=c){
      c[0]=a[0];c[1]=a[1];c[2]=a[2];c[3]=a[3];c[12]=a[12];c[13]=a[13];c[14]=a[14];c[15]=a[15]
    }
  } else c=a;
  c[4]=e*b+i*d;c[5]=g*b+j*d;c[6]=f*b+k*d;c[7]=h*b+l*d;c[8]=e*-d+i*b;c[9]=g*-d+j*b;c[10]=f*-d+k*b;c[11]=h*-d+l*b;
  return c
};

mat4.rotateY=function(a,b,c){
  var d=Math.sin(b);b=Math.cos(b);
  var e=a[0],g=a[1],f=a[2],h=a[3],i=a[8],j=a[9],k=a[10],l=a[11];
  if(c){
    if(a!=c){
      c[4]=a[4];c[5]=a[5];c[6]=a[6];c[7]=a[7];c[12]=a[12];c[13]=a[13];c[14]=a[14];c[15]=a[15]
    }
  }else c=a;
  c[0]=e*b+i*-d;c[1]=g*b+j*-d;c[2]=f*b+k*-d;c[3]=h*b+l*-d;c[8]=e*d+i*b;c[9]=g*d+j*b;c[10]=f*d+k*b;c[11]=h*d+l*b;
  return c
};

mat4.rotateZ=function(a,b,c){
  var d=Math.sin(b);b=Math.cos(b);
  var e=a[0],g=a[1],f=a[2],h=a[3],i=a[4],j=a[5],k=a[6],l=a[7];
  if(c){
    if(a!=c){c[8]=a[8];c[9]=a[9];c[10]=a[10];c[11]=a[11];c[12]=a[12];c[13]=a[13];c[14]=a[14];c[15]=a[15]
      }
    }else c=a;
    c[0]=e*b+i*d;c[1]=g*b+j*d;c[2]=f*b+k*d;c[3]=h*b+l*d;c[4]=e*-d+i*b;c[5]=g*-d+j*b;c[6]=f*-d+k*b;c[7]=h*-d+l*b;
    return c
  };

mat4.frustum=function(a,b,c,d,e,g,f){
  f||(f=mat4.create());
  var h=b-a,i=d-c,j=g-e;f[0]=e*2/h;f[1]=0;f[2]=0;f[3]=0;f[4]=0;f[5]=e*2/i;f[6]=0;f[7]=0;f[8]=(b+a)/h;f[9]=(d+c)/i;f[10]=-(g+e)/j;f[11]=-1;f[12]=0;f[13]=0;f[14]=-(g*e*2)/j;f[15]=0;
  return f
};

mat4.perspective=function(a,b,c,d,e){
  a=c*Math.tan(a*Math.PI/360);b=a*b;return mat4.frustum(-b,b,-a,a,c,d,e)
};

mat4.ortho=function(a,b,c,d,e,g,f){
  f||(f=mat4.create());
  var h=b-a,i=d-c,j=g-e;f[0]=2/h;f[1]=0;f[2]=0;f[3]=0;f[4]=0;f[5]=2/i;f[6]=0;f[7]=0;f[8]=0;f[9]=0;f[10]=-2/j;f[11]=0;f[12]=-(a+b)/h;f[13]=-(d+c)/i;f[14]=-(g+e)/j;f[15]=1;
  return f
};

mat4.lookAt=function(a,b,c,d){
  d||(d=mat4.create());
  var e=a[0],g=a[1];a=a[2];
  var f=c[0],h=c[1],i=c[2];c=b[1];
  var j=b[2];
  if(e==b[0]&&g==c&&a==j)
    return mat4.identity(d);
  var k,l,o,m;c=e-b[0];j=g-b[1];b=a-b[2];m=1/Math.sqrt(c*c+j*j+b*b);c*=m;j*=m;b*=m;k=h*b-i*j;i=i*c-f*b;f=f*j-h*c;
  if(m=Math.sqrt(k*k+i*i+f*f)){
    m=1/m;k*=m;i*=m;f*=m
  }else f=i=k=0;
  h=j*f-b*i;l=b*k-c*f;o=c*i-j*k;
  if(m=Math.sqrt(h*h+l*l+o*o)){
    m=1/m;h*=m;l*=m;o*=m
  }else o=l=h=0;
  d[0]=k;d[1]=h;d[2]=c;d[3]=0;d[4]=i;d[5]=l;d[6]=j;d[7]=0;d[8]=f;d[9]=o;d[10]=b;d[11]=0;d[12]=-(k*e+i*g+f*a);d[13]=-(h*e+l*g+o*a);d[14]=-(c*e+j*g+b*a);d[15]=1;
  return d
};

mat4.str=function(a){
  return"["+a[0]+", "+a[1]+", "+a[2]+", "+a[3]+", "+a[4]+", "+a[5]+", "+a[6]+", "+a[7]+", "+a[8]+", "+a[9]+", "+a[10]+", "+a[11]+", "+a[12]+", "+a[13]+", "+a[14]+", "+a[15]+"]"
};


// Perspective Projection
mat4.my_perspectiv = function ( fov, aspectRatio, near, far, m ) {
  var r = aspectRatio;
  var fn = far + near;
  var f_n = far - near;
  var t = 1.0 / Math.tan( fov / 2 );
  m[0] = t/r; m[1] = 0; m[2] = 0; m[3] = 0;
  m[4] = 0; m[5] = t; m[6] = 0; m[7] = 0;
  m[8] = 0; m[9] = 0; m[10] = -fn/f_n; m[11] = -2*far*near/f_n;
  m[12] = 0; m[13] = 0; m[14] = -1; m[15] = 0;
  return m
};

// Orthographic Projection
mat4.my_ortho = function ( fov, aspectRatio, near, far, m ) {
  var r = aspectRatio;
  var fn = far + near;
  var f_n = far - near;
  var t = 1.0 / Math.tan( fov / 2 );
  m[0] = t/r; m[1] = 0; m[2] = 0; m[3] = 0;
  m[4] = 0; m[5] = t; m[6] = 0; m[7] = 0;
  m[8] = 0; m[9] = 0; m[10] = -2.0/f_n; m[11] = -fn/f_n;
  m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
  return m
};


//! /class gl
//!
//!
var gl = null;


//! /class gl_util
//!
//!
var gl_util = {

  //! general initilaizations

  detectIE: function() {
      var ua = window.navigator.userAgent;

      var msie = ua.indexOf('MSIE ');
      if (msie > 0) {
          // IE 10 or older => return version number
          return parseInt(ua.substring(msie + 5, ua.indexOf('.', msie)), 10);
      }

      var trident = ua.indexOf('Trident/');
      if (trident > 0) {
          // IE 11 => return version number
          var rv = ua.indexOf('rv:');
          return parseInt(ua.substring(rv + 3, ua.indexOf('.', rv)), 10);
      }

      /*
      var edge = ua.indexOf('Edge/');
      if (edge > 0) {
          // IE 12 => return version number
          return parseInt(ua.substring(edge + 5, ua.indexOf('.', edge)), 10);
      }
      */

      // other browser
      return 0;
  },

  isIE: function() {
    return this.detectIE() > 0;
  },

  init : function( canvas, useDepthTexExt ) {

    // "2d", leading to the creation of a CanvasRenderingContext2D object representing a two-dimensional rendering context.
    //"webgl" (or "experimental-webgl") which will create a WebGLRenderingContext object representing a three-dimensional rendering context.
    //        This context is only available on browsers that implement WebGL version 1 (OpenGL ES 2.0).
    //"webgl2" (or "experimental-webgl2") which will create a WebGL2RenderingContext object representing a three-dimensional rendering context. 
    //        This context is only available on browsers that implement WebGL version 2 (OpenGL ES 3.0). 
    //"bitmaprenderer" which will create a ImageBitmapRenderingContext which only provides functionality to replace the content of the canvas
    //        with a given ImageBitmap.
 
    var exceptMsg = false;
    try {
      gl = canvas.getContext( "experimental-webgl" );
    } catch(e) {
      if ( e.message ) {
        exceptMsg = true;
        alert("Could not initialise WebGL: " + e.message);
      }
    }
    if (exceptMsg == false && !gl) {
      alert("Could not initialise WebGL!");
    }
    if (!gl)
      return null;

    gl.viewportWidth = canvas.width;
    gl.viewportHeight = canvas.height;

    var standard_derivatives = gl.getExtension( "OES_standard_derivatives" );  // dFdx, dFdy
    if (!standard_derivatives) {
      alert('no standard derivatives support (no dFdx, dFdy)');
    }
    if ( useDepthTexExt && useDepthTexExt != 0 ) {
      var depth_texture_extension = gl.getExtension( "WEBKIT_WEBGL_depth_texture" );  // gl.DEPTH_COMPONENT
      if (!standard_derivatives) {
        alert('no depth buffer texture support (no gl.DEPTH_COMPONENT)');
      }
    }
    //gl.getExtension( "EXT_shader_texture_lod" );
    //var floatTextures = gl.getExtension('OES_texture_float');
    //if (!floatTextures) {
    //  alert('no floating point texture support');
    //}

    return gl;
  },

  //! actiate gl canvas

  activateCanvas : function( gl_canvas ) {
    gl = gl_canvas;
    return gl;
  },


  //! shader program

  getShaderFromStr : function( code, type, silent )
  {
    if ( code == undefined || type == undefined )
      return;
    var codeStr = "";
    if ( Array.isArray( code ) ) {
      for ( var i_c = 0; i_c < code.length; ++ i_c ) {
        codeStr += code[i_c];
        codeStr += "\n";
      } 
    } else {
      codeStr += code;
    }
    var shader = gl.createShader( type );
    gl.shaderSource(shader, codeStr);
    gl.compileShader(shader);
    if ( !gl.getShaderParameter(shader, gl.COMPILE_STATUS) ) {
      if ( !silent || silent==false ) alert(gl.getShaderInfoLog(shader));
      return;
    } 
    return shader;  
  },

  getShader : function( data, silent ) {
    var shaderScript = document.getElementById( data );
    var typeStr = "";
    var str = "";
    if ( shaderScript ) {
      var k = shaderScript.firstChild;
      while (k) {
        if (k.nodeType == 3) {
          str += k.textContent;
        }
        k = k.nextSibling;
      }
      typeStr = shaderScript.type;
    } else if ( data.code ) {
      str = data.code;
      typeStr = data.type;
    } else {
      str = data;
      typeStr = data.type;
    }
    var type;
    if ( typeStr == "x-shader/x-fragment" || typeStr == "frag" ) {
      type = gl.FRAGMENT_SHADER;
    } else if ( typeStr == "x-shader/x-vertex"  || typeStr == "vert" ) {
      type = gl.VERTEX_SHADER;
    } else {
      return;
    }
    return this.getShaderFromStr( str, type, silent );
  },

  useProgram : function( shaderProgram ) {
    gl.useProgram(shaderProgram);
    return shaderProgram;
  },

  deleteProgram : function( shaderProgram ) {
    gl.deleteProgram(shaderProgram);
  },

  use_new_vs_fs : function( vs, fs, silent ) {
    var vertexShader = this.getShader( vs, silent );
    var fragmentShader = this.getShader( fs, silent );
    if ( !vertexShader || !fragmentShader ) {
      return;
    }
    var shaderProgram = gl.createProgram();
    gl.attachShader(shaderProgram, vertexShader);
    gl.attachShader(shaderProgram, fragmentShader);
    gl.linkProgram(shaderProgram);
    var status = gl.getProgramParameter(shaderProgram, gl.LINK_STATUS );
    if ( !status ) {
      if (!silent || silent==false) {
        alert( "Could not initialise shaders!" );
      }
      return;
    }
    this.useProgram(shaderProgram);
    return shaderProgram;
  },


  //! element and array buffer

 
  VERTEX : 0,
  NORMAL : 1,
  TANGENT : 2,
  BINORMAL : 3,
  TEXTURE : 4,
  ELEMENT : 5,
  LINE : 6,

  createEmptyBuffer : function( type ) {
    var buf = gl.createBuffer();
    buf.type = type;
    return buf;
  },

  bindBufferData : function( buf, numItems, itemSize, bufferArr, elementKind ) {
    
    if ( buf.type != this.ELEMENT ) {
      gl.bindBuffer( gl.ARRAY_BUFFER, buf );
      gl.bufferData( gl.ARRAY_BUFFER, new Float32Array( bufferArr ), gl.STATIC_DRAW );
      buf.itemSize = itemSize;
    } else {
      gl.bindBuffer( gl.ELEMENT_ARRAY_BUFFER, buf );
      gl.bufferData( gl.ELEMENT_ARRAY_BUFFER, new Uint16Array( bufferArr ), gl.STATIC_DRAW );
      buf.elementKind = elementKind;
    }
    buf.numItems = numItems;
    return buf;
  },
  
  createBuffer : function( type, numItems, itemSize, bufferArr, elementKind ) {
    var buf = this.createEmptyBuffer( type );
    this.bindBufferData( buf, numItems, itemSize, bufferArr, elementKind );
    return buf;
  },

  linkBufferLocation : function( shaderProgram, name ) {
    var loc = gl.getAttribLocation( shaderProgram, name );
    return loc;
  },

  bindBufferToLocation : function( buf, loc ) {
    gl.enableVertexAttribArray( loc );
    gl.bindBuffer( gl.ARRAY_BUFFER, buf );
    gl.vertexAttribPointer( loc, buf.itemSize, gl.FLOAT, false, 0, 0 );
  },


  
  //! texture

  defaultTexture2DParameter : function() {
    gl.texParameteri( gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR );
    gl.texParameteri( gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR );
    gl.texParameteri( gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.REPEAT );
  	gl.texParameteri( gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.REPEAT );
    //gl.texParameteri( gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_NEAREST );
    //gl.generateMipmap( gl.TEXTURE_2D );
  },

  newTexture2D : function( unitInx, width, height, data, flipY ) {
    var texture = gl.createTexture();
    gl.activeTexture( gl.TEXTURE0 + unitInx );
    gl.bindTexture( gl.TEXTURE_2D, texture );
    gl.texImage2D( gl.TEXTURE_2D, 0, gl.RGBA, width, height, 0, gl.RGBA, gl.UNSIGNED_BYTE, data );
    if ( flipY != undefined && flipY == true )
      gl.pixelStorei( gl.UNPACK_FLIP_Y_WEBGL, true );
    this.defaultTexture2DParameter();
    if ( unitInx == 0 )
      gl.bindTexture( gl.TEXTURE_2D, null );
    return texture;
  },

  handleLoadedTexture2D : function( image, texture, flipY ) {
    gl.activeTexture( gl.TEXTURE0 );
    gl.bindTexture( gl.TEXTURE_2D, texture );
    gl.texImage2D( gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, image );
    if ( flipY != undefined && flipY == true )
      gl.pixelStorei( gl.UNPACK_FLIP_Y_WEBGL, true );
    this.defaultTexture2DParameter();
    gl.bindTexture( gl.TEXTURE_2D, null );
    return texture;
  },

  loadTexture2D : function( name ) {
    var texture = gl.createTexture();
    texture.image = new Image();
    texture.image.setAttribute('crossorigin', 'anonymous');
    //texture.image = document.createElement('img');
    texture.image.onload = function () {
        gl_util.handleLoadedTexture2D( texture.image, texture )
    }
    texture.image.src = name;
    return texture;
  }, 

  ssaoTextures : function( ssaoObject, kernelSize, noiseSize ) {
    if ( !ssaoObject ){
      ssaoObject = new Object();
    }
    
    ssaoObject.kernel = new Uint8Array( kernelSize * 4 );
    for ( var i_k = 0; i_k < kernelSize; ++ i_k ) {
      var x = Math.random() * 2.0 - 1.0;
      var y = Math.random() * 2.0 - 1.0;
      var z = Math.random() * 2.0 - 1.0;
      var len = Math.sqrt( x * x + y * y + z * z );
      var w = i_k / kernelSize;
      w = 0.1 + 0.9 * w * w;
      ssaoObject.kernel[i_k*4] = ( x / len + 1.0 ) * 0.5 * 255.0;
      ssaoObject.kernel[i_k*4+1] = ( y / len + 1.0 ) * 0.5 * 255.0;
      ssaoObject.kernel[i_k*4+2] = ( z / len + 1.0 ) * 0.5 * 255.0;
      ssaoObject.kernel[i_k*4+3] = w * 255.0;
    }

    ssaoObject.noise = new Uint8Array( noiseSize * noiseSize * 4 );
    for ( var i_n = 0; i_n < noiseSize * noiseSize; ++ i_n ) {
      var x = Math.random() * 2.0 - 1.0;
      var y = Math.random() * 2.0 - 1.0;
      var z = Math.random() * 2.0 - 1.0;
      var len = Math.sqrt( x * x + y * y + z * z );
      ssaoObject.noise[i_n*4] = ( x / len + 1.0 ) * 0.5 * 255.0;
      ssaoObject.noise[i_n*4+1] = ( y / len + 1.0 ) * 0.5 * 255.0;
      ssaoObject.noise[i_n*4+2] = ( z / len + 1.0 ) * 0.5 * 255.0;
      ssaoObject.noise[i_n*4+3] = 255.0;
    }

    gl.activeTexture( gl.TEXTURE0 );
    
    if ( !ssaoObject.kernelTexture ) {
      ssaoObject.kernelTexture = gl.createTexture();
    }
    gl.bindTexture( gl.TEXTURE_2D, ssaoObject.kernelTexture );
    gl.texImage2D( gl.TEXTURE_2D, 0, gl.RGBA, kernelSize, 1, 0, gl.RGBA, gl.UNSIGNED_BYTE, ssaoObject.kernel );
    gl.texParameteri( gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST );
    gl.texParameteri( gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST );
    gl.texParameteri( gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE );
  	gl.texParameteri( gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE );
    
    if ( !ssaoObject.noiseTexture ) {
      ssaoObject.noiseTexture = gl.createTexture();
    }
    gl.bindTexture( gl.TEXTURE_2D, ssaoObject.noiseTexture );
    gl.texImage2D( gl.TEXTURE_2D, 0, gl.RGBA, noiseSize, noiseSize, 0, gl.RGBA, gl.UNSIGNED_BYTE, ssaoObject.noise );
    gl.texParameteri( gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST );
    gl.texParameteri( gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST );
    gl.texParameteri( gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE );
  	gl.texParameteri( gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE );

    gl.bindTexture( gl.TEXTURE_2D, null );

    return ssaoObject;
  },

  useTexture2D : function( unitInx, id, sampler ) {
    gl.activeTexture( gl.TEXTURE0 + unitInx );
    gl.bindTexture( gl.TEXTURE_2D, id );
    gl.uniform1i( sampler, unitInx );
  },

  //! frame buffer and render buffer

  deleteFB : function( fb ) {
    if ( fb == undefined )
      return;
    if ( fb.depth_texture ) {
      gl.deleteTexture( fb.depth_texture );
    }
    if ( fb.color0_texture ) {
      gl.deleteTexture( fb.color0_texture  );
    }
    gl.deleteFramebuffer( fb );
  },

  createTextureFB : function( addDepthBufferTexture, fb_width, fb_height, color_plane ) {
    
    var useDepthTexture = addDepthBufferTexture && addDepthBufferTexture != 0;

    var fb = gl.createFramebuffer();
    gl.bindFramebuffer( gl.FRAMEBUFFER, fb );
    fb.width = ( fb_width && fb_width != 0 ) ? fb_width : gl.viewportWidth;
    fb.height = ( fb_height && fb_height != 0 ) ? fb_width : gl.viewportHeight;

    // create the color texture
    if ( color_plane == undefined ) {
      fb.color0_texture = gl.createTexture();
      gl.bindTexture( gl.TEXTURE_2D, fb.color0_texture );
      gl.texParameteri( gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST );
      gl.texParameteri( gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST );
      gl.texImage2D( gl.TEXTURE_2D, 0, gl.RGBA, fb.width, fb.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null );
    } else {
      fb.color0_texture = color_plane;
    }

    if ( useDepthTexture == 1 ) {
      // create the depth texture
      fb.depth_texture = gl.createTexture();
      gl.bindTexture( gl.TEXTURE_2D, fb.depth_texture );
      gl.texParameteri( gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST );
      gl.texParameteri( gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST );
      gl.texImage2D( gl.TEXTURE_2D, 0, gl.DEPTH_COMPONENT, fb.width, fb.height, 0, gl.DEPTH_COMPONENT, gl.UNSIGNED_SHORT, null );
      //gl.texImage2D( gl.TEXTURE_2D, 0, gl.DEPTH_STENCIL, fb.width, fb.height, 0, gl.DEPTH_STENCIL, gl.UNSIGNED_SHORT, null );
    } else {
      // create the render buffer
      fb.renderbuffer = gl.createRenderbuffer();
      gl.bindRenderbuffer( gl.RENDERBUFFER, fb.renderbuffer );
      gl.renderbufferStorage( gl.RENDERBUFFER, gl.DEPTH_COMPONENT16, fb.width, fb.height );
    }

    // bind color texture to frame buffer
    gl.framebufferTexture2D( gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, fb.color0_texture, 0 );
    // bind depth texture or render buffer to frame buffer
    if ( useDepthTexture == 1 ) {
      gl.framebufferTexture2D( gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.TEXTURE_2D, fb.depth_texture, 0 );
      //gl.framebufferTexture2D( gl.FRAMEBUFFER, gl.STENCIL_ATTACHMENT, gl.TEXTURE_2D, fb.depth_texture, 0 );
    } else {
      gl.framebufferRenderbuffer( gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.RENDERBUFFER, fb.renderbuffer );
    }

    gl.bindTexture( gl.TEXTURE_2D, null );
    gl.bindRenderbuffer( gl.RENDERBUFFER, null );
    gl.bindFramebuffer( gl.FRAMEBUFFER, null );

    return fb;
  },

};


//! /class mesh
//!
//!
var mesh={};

mesh.SOLID = 0;
mesh.WIRE = 1;

mesh.createBuffer = function( me ) {
  me.vertexBuffer_sp = gl_util.createBuffer( gl_util.VERTEX, me.numArr, me.vertexArr.itemSize, me.vertexArr, 0 );
  me.normalBuffer_sp = gl_util.createBuffer( gl_util.NORMAL, me.numArr, me.normalArr.itemSize, me.normalArr, 0 );
  if ( me.textureArr )
    me.textureBuffer_sp = gl_util.createBuffer( gl_util.TEXTURE, me.numArr, me.textureArr.itemSize, me.textureArr, 0 );
  if ( me.skinIndexArr )
    me.skinIndexBuffer_sp = gl_util.createBuffer( gl_util.TEXTURE, me.numArr, me.skinIndexArr.itemSize, me.skinIndexArr, 0 );
  if ( me.skinWeightArr )
    me.skinWeightBuffer_sp = gl_util.createBuffer( gl_util.TEXTURE, me.numArr, me.skinWeightArr.itemSize, me.skinWeightArr, 0 );
  me.elementBuffer_sp = gl_util.createBuffer( gl_util.ELEMENT, me.numElemArr, me.elementArr.itemSize, me.elementArr, me.elementKind ); 
}

mesh.draw = function( me, v, n, t, si, sw ) {
  gl_util.bindBufferToLocation( me.vertexBuffer_sp, v );
  gl_util.bindBufferToLocation( me.normalBuffer_sp, n );
  if ( t && me.textureBuffer_sp )
    gl_util.bindBufferToLocation( me.textureBuffer_sp, t );
  if ( si && me.skinIndexBuffer_sp )  
    gl_util.bindBufferToLocation( me.skinIndexBuffer_sp, si );
  if ( sw && me.skinWeightBuffer_sp )
    gl_util.bindBufferToLocation( me.skinWeightBuffer_sp, sw );
  
  gl.bindBuffer( gl.ELEMENT_ARRAY_BUFFER, me.elementBuffer_sp );
  gl.drawElements( me.elementBuffer_sp.elementKind, me.elementBuffer_sp.numItems, gl.UNSIGNED_SHORT, 0 );

  gl.disableVertexAttribArray( me.vertexBuffer_sp );
  gl.disableVertexAttribArray( me.normalBuffer_sp );
  if ( t && me.textureBuffer_sp )
    gl.disableVertexAttribArray( me.textureBuffer_sp );
  if ( si && me.skinIndexBuffer_sp ) 
    gl.disableVertexAttribArray( me.skinIndexBuffer_sp );
  if ( sw && me.skinWeightBuffer_sp )
    gl.disableVertexAttribArray( me.skinWeightBuffer_sp );
}

mesh.newMesh = function( drawType, elementKind ) {
  var me = new Object();
  me.skin = 0;
  me.drawType = drawType ? drawType : mesh.SOLID;

  me.vertexArr = [];
  me.vertexArr.itemSize = 3;
  me.normalArr = [];
  me.normalArr.itemSize = 3;
  me.textureArr = [];
  me.textureArr.itemSize = 2;
  me.numArr = 0;
    
  me.elementArr = [];
  me.elementArr.itemSize = drawType == this.WIRE ? 2 : 3;
  me.elementKind = elementKind ? elementKind : (drawType == this.WIRE ? gl.LINES : gl.TRIANGLES);
  me.numElemArr = 0;

  return me;
}

mesh.addV = function( me, x, y, z, nx, ny, nz, u, v ) {
  me.vertexArr.push( x, y, z );
  me.normalArr.push( nx, ny, nz );
  me.textureArr.push( u, v );
  me.numArr ++;
};

mesh.addTriangle = function( me, a, b, c ) {
  if ( me.drawType == mesh.WIRE ) {
    me.elementArr.push( a, b, b, c, c, a );
    me.numElemArr += 6;
  } else {
    me.elementArr.push( a, b, c );
    me.numElemArr += 3;
  }
};

mesh.addQuad = function( me, a, b, c, d ) {
  if ( me.drawType == mesh.WIRE ) {
    me.elementArr.push( a, b, b, c, c, d, d, a );
    me.numElemArr += 8;
  } else {
    me.elementArr.push( a, b, c, a, c, d );
    me.numElemArr += 6;
  }
};

mesh.createQuad = function() {
  var me = mesh.newMesh( mesh.SOLID, gl.TRIANGLE_STRIP );
  mesh.addV( me, 1.0, 1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0 );
  mesh.addV( me, -1.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0 );
  mesh.addV( me, 1.0, -1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0 );
  mesh.addV( me, -1.0, -1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0 );
  
  me.elementArr.push( 0, 1, 2, 3 );
  me.numElemArr += 4;
  
  return me;
}

mesh.createRect =  function( rows, columns, drawType ) {
  var me = mesh.newMesh( drawType );

  for ( var inxY = 0; inxY <= rows; ++ inxY ) {
    for ( var inxX = 0; inxX <= columns; ++ inxX ) {
      var x = inxX / columns;
      var y = inxY / rows;
      mesh.addV( me, x * 2.0 - 1.0, y * 2.0 - 1.0, 0, 0, 0, 1, x, y );
    }
  }

  for ( var inxY = 0; inxY < rows; ++ inxY ) {
    for ( var inxX = 0; inxX < columns; ++ inxX ) {
      var row1 = (columns+1) * inxY;
      var row2 = (columns+1) * (inxY+1);
      mesh.addQuad( me, row1 + inxX, row2 + inxX, row2 + inxX + 1, row1 + inxX + 1 );
    }
  }
      
  return me;
};

mesh.createSphere = function( circumferenceCount, topToBottomeCount, drawType ) {
  var me = mesh.newMesh( drawType );
  var circumferenceCount_2 = circumferenceCount / 2;
  mesh.addV( me, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.5, 1.0 );
  for ( var tbInx = 1; tbInx < topToBottomeCount; tbInx ++ )
  {
    var v = 1.0 - tbInx / topToBottomeCount;
    var heightFac = Math.sin( ( 1.0 - 2.0 * tbInx / topToBottomeCount ) * Math.PI * 0.5 );
    var cosUp = Math.sqrt( 1.0 - heightFac * heightFac );
    var z = heightFac;
    for ( var cInx = 0; cInx <= circumferenceCount_2; cInx ++ )
    {
      var u = cInx / circumferenceCount_2;
      var angle =  Math.PI * u;
      var x = Math.cos( angle ) * cosUp;
      var y = Math.sin( angle ) * cosUp;
      mesh.addV( me, x, y, z, x, y, z, u, v );
    }
    for ( cInx = 0; cInx <= circumferenceCount_2; cInx ++ )
    {
      var u = cInx / circumferenceCount_2;
      var angle = Math.PI * u + Math.PI;
      var x = Math.cos( angle ) * cosUp;
      var y = Math.sin( angle ) * cosUp;
      mesh.addV( me, x, y, z, x, y, z, u, v );
    }
  }
  mesh.addV( me, 0.0, 0.0, -1.0, 0.0, 0.0, -1.0, 0.5, 0.0 );

  var circumferenceSize_2 = circumferenceCount_2 + 1;
  var circumferenceSize = circumferenceSize_2 * 2;
  for ( var cInx = 0; cInx < circumferenceCount_2; cInx ++ )
    mesh.addTriangle( me, cInx + 1, cInx + 2, 0 );
  for ( var cInx = 0; cInx < circumferenceCount_2; cInx ++ )
    mesh.addTriangle( me, circumferenceSize_2 + cInx + 1, circumferenceSize_2 + cInx + 2, 0 );
  for ( var tbInx = 1; tbInx < topToBottomeCount - 1; tbInx ++ )
  {
    var ringStart = 1 + (tbInx - 1) * circumferenceSize;
    var nextRingStart = 1 + tbInx * circumferenceSize;
    for ( var cInx = 0; cInx < circumferenceCount_2; cInx ++ )
      mesh.addQuad( me, ringStart + cInx, nextRingStart + cInx, nextRingStart + cInx + 1, ringStart + cInx + 1 );
    ringStart += circumferenceSize_2;
    nextRingStart += circumferenceSize_2;
    for ( var cInx = 0; cInx < circumferenceCount_2; cInx ++ )
      mesh.addQuad( me, ringStart + cInx, nextRingStart + cInx, nextRingStart + cInx + 1, ringStart + cInx + 1 );
  }
  var ringPtCount = ( topToBottomeCount - 1 ) * circumferenceSize;
  var start = 1 + ringPtCount - circumferenceSize;
  for ( var cInx = 0; cInx < circumferenceCount_2; cInx ++ )
    mesh.addTriangle( me, start + cInx + 1, start + cInx, 1 + ringPtCount );
  start += circumferenceSize_2;
  for ( var cInx = 0; cInx < circumferenceCount_2; cInx ++ )
    mesh.addTriangle( me, start + cInx + 1, start + cInx, 1 + ringPtCount );
    
  return me;
};

mesh.createTorus = function( sides, rings, radius, width, drawType ) {
  var me = mesh.newMesh( drawType );

  var ringFactor = 2.0 * Math.PI / rings;
  var sideFactor = 2.0 * Math.PI / sides;
  var idx = 0;
  var tidx = 0;
  for ( var ring = 0; ring <= rings; ++ ring )
  {
    var u = ring * ringFactor;
    var cu = Math.cos(u);
    var su = Math.sin(u);
    for ( var side = 0; side <= sides; ++ side )
    {
      var v = side * sideFactor;
      var cv = -Math.cos( v );
      var sv = -Math.sin( v );
      var r = radius + width * cv;
      mesh.addV( me, r * cu, r * su, width * sv, cv * cu * r, cv * su * r, sv * r, u / Math.PI / 2.0, v / Math.PI / 2.0 );
    }
  }

  idx = 0;
  for ( var ring = 0; ring < rings; ++ ring )
  {
    var ringStart = ring * (sides + 1);
    var nextRingStart = ringStart + sides + 1;
    for ( var side = 0; side < sides; ++ side )
    {
      var nextSide = side + 1;
      mesh.addQuad( me, ringStart + side, nextRingStart + side, nextRingStart + nextSide, ringStart + nextSide );
    }
  }

  return me;
}

mesh.splitLine = function( me, i0, i1, separateNormals ) {
  var newPt = [ ( me.vertexArr[i0*3] + me.vertexArr[i1*3] ) / 2.0, ( me.vertexArr[i0*3+1] + me.vertexArr[i1*3+1] ) / 2.0, ( me.vertexArr[i0*3+2] + me.vertexArr[i1*3+2] ) / 2.0 ];
  var newNV = [ ( me.normalArr[i0*3] + me.normalArr[i1*3] ) / 2.0, ( me.normalArr[i0*3+1] + me.normalArr[i1*3+1] ) / 2.0, ( me.normalArr[i0*3+2] + me.normalArr[i1*3+2] ) / 2.0 ];
  var newTex = [ ( me.textureArr[i0*2] + me.textureArr[i1*2] ) / 2.0, ( me.textureArr[i0*2+1] + me.textureArr[i1*2+1] ) / 2.0 ];
  var lenPt = Math.sqrt( newPt[0] * newPt[0] + newPt[1] * newPt[1] + newPt[2] * newPt[2] );
  newPt[0] /= lenPt;
  newPt[1] /= lenPt;
  newPt[2] /= lenPt;
  var lenNV = Math.sqrt( newNV[0] * newNV[0] + newNV[1] * newNV[1] + newNV[2] * newNV[2] );
  newNV[0] /= lenNV;
  newNV[1] /= lenNV;
  newNV[2] /= lenNV;
  mesh.addV( me, newPt[0], newPt[1], newPt[2], newNV[0], newNV[1], newNV[2], newTex[0], newTex[1] );
  return me;
}

mesh.splitTriangle1 = function( me, separateNormals ) {
      
  var faces = me.elementArr;
  var noOfIndices = me.numElemArr;   
  me.elementArr = [];
  me.numElemArr = 0;

  for( var faceStart = 0; faceStart < noOfIndices; faceStart = faceStart + 3 ) {
    var f = [ faces[faceStart], faces[faceStart+1], faces[faceStart+2] ];
    var nf = [ me.numArr, me.numArr+1, me.numArr+2 ];
    me = mesh.splitLine( me, f[0], f[1], separateNormals );
    me = mesh.splitLine( me, f[1], f[2], separateNormals );
    me = mesh.splitLine( me, f[2], f[0], separateNormals );
    mesh.addTriangle( me, f[0], nf[0], nf[2] );
    mesh.addTriangle( me, f[1], nf[1], nf[0] );
    mesh.addTriangle( me, f[2], nf[2], nf[1] );
    mesh.addTriangle( me, nf[0], nf[1], nf[2] );
  }

  return me;
}


mesh.splitTriangle2 = function( me, separateNormals ) {
      
  var faces = me.elementArr;
  var noOfIndices = me.numElemArr;   
  me.elementArr = [];
  me.numElemArr = 0;

  for( var faceStart = 0; faceStart < noOfIndices; faceStart = faceStart + 3 ) {
    var f = [ faces[faceStart], faces[faceStart+1], faces[faceStart+2] ];
    var nf = me.numArr;
    var newPt = [ ( me.vertexArr[f[0]*3] + me.vertexArr[f[1]*3] + me.vertexArr[f[2]*3] ) / 3.0,
                  ( me.vertexArr[f[0]*3+1] + me.vertexArr[f[1]*3+1] + me.vertexArr[f[2]*3+1] ) / 3.0,
                  ( me.vertexArr[f[0]*3+2] + me.vertexArr[f[1]*3+2] + me.vertexArr[f[2]*3+2] ) / 3.0 ];
    var newNV = [ ( me.normalArr[f[0]*3] + me.normalArr[f[1]*3] + me.normalArr[f[2]*3] ) / 3.0,
                  ( me.normalArr[f[0]*3+1] + me.normalArr[f[1]*3+1] + me.normalArr[f[2]*3+1] ) / 3.0,
                  ( me.normalArr[f[0]*3+2] + me.normalArr[f[1]*3+2] + me.normalArr[f[2]*3+2] ) / 3.0 ];
    var newTex = [ ( me.textureArr[f[0]*3] + me.textureArr[f[1]*3] + me.textureArr[f[2]*3] ) / 3.0,
                   ( me.textureArr[f[0]*3+1] + me.textureArr[f[1]*3+1] + me.textureArr[f[2]*3+1] ) / 3.0 ];
    var lenPt = Math.sqrt( newPt[0] * newPt[0] + newPt[1] * newPt[1] + newPt[2] * newPt[2] );
    newPt[0] /= lenPt;
    newPt[1] /= lenPt;
    newPt[2] /= lenPt;
    var lenNV = Math.sqrt( newNV[0] * newNV[0] + newNV[1] * newNV[1] + newNV[2] * newNV[2] );
    newNV[0] /= lenNV;
    newNV[1] /= lenNV;
    newNV[2] /= lenNV;
    mesh.addV( me, newPt[0], newPt[1], newPt[2], newNV[0], newNV[1], newNV[2], newTex[0], newTex[1] );
    mesh.addTriangle( me, f[0], f[1], nf );
    mesh.addTriangle( me, f[1], f[2], nf );
    mesh.addTriangle( me, f[2], f[0], nf );
  }

  return me;
}


mesh.createTetrahedron = function( drawType ) {
  var me = mesh.newMesh( drawType );

  var c_sin120 = 0.8660254;
  mesh.addV( me, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0 );
  mesh.addV( me, 0.0, -c_sin120, -0.5, 0.0, -c_sin120, -0.5, 0.0, 0.0 );
  mesh.addV( me, c_sin120 * c_sin120, 0.5 * c_sin120, -0.5, c_sin120 * c_sin120, 0.5 * c_sin120, -0.5, 1.0, 0.0 );
  mesh.addV( me, -c_sin120 * c_sin120, 0.5 * c_sin120, -0.5, -c_sin120 * c_sin120, 0.5 * c_sin120, -0.5, 0.0, 1.0 );

  mesh.addTriangle( me, 0, 1, 2 );
  mesh.addTriangle( me, 0, 2, 3 );
  mesh.addTriangle( me, 0, 3, 1 );
  mesh.addTriangle( me, 1, 3, 2 );

  return me;
}

mesh.createTriangleSphere = function( minPts, drawType ) {
  var me = mesh.createTetrahedron( drawType );
  while ( me.numArr < minPts )
    me = mesh.splitTriangle1( me, 0 );
  return me;
}

mesh.createTriangleObject1 = function( minPts, drawType ) {
  var me = mesh.createTetrahedron( drawType );
  while ( me.numArr < minPts )
    me = mesh.splitTriangle2( me, 1 );
  return me;
}

mesh.createArrowVolume = function( arrowLen, arrowRad, peakLen, peakRad, res ) {
  var me = mesh.newMesh( mesh.SOLID, gl.TRIANGLES );

  mesh.addV( me, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0 );
  for ( var inx = 0; inx <= res; ++ inx ) {
    var x = Math.cos( Math.PI * 2.0 * inx / res );
    var y = Math.sin( Math.PI * 2.0 * inx / res );
    mesh.addV( me, x * peakRad, y * peakRad, -peakLen, x, y, 0.0, 0.0, inx / res );
  }
  for ( var inx = 0; inx <= res; ++ inx ) {
    var x = Math.cos( Math.PI * 2.0 * inx / res );
    var y = Math.sin( Math.PI * 2.0 * inx / res );
    mesh.addV( me, x * arrowRad, y * arrowRad, -peakLen, x, y, 0.0, 1.0, inx / res );
    mesh.addV( me, x * arrowRad, y * arrowRad, -arrowLen, x, y, 0.0, 0.0, inx / res );
  }
  mesh.addV( me, 0.0, 0.0, -arrowLen, 0.0, 0.0, -1.0, 1.0, 1.0 );
  
  
  for ( var inx = 0; inx < res; ++ inx ) {
   mesh.addTriangle( me, inx + 1, inx + 2, 0 );
  }
  for ( var inx = 0; inx < res; ++ inx ) {
    mesh.addQuad( me, inx+1, res+(inx*2)+2, res+(inx*2)+4, inx+2 );
  }
  for ( var inx = 0; inx < res; ++ inx ) {
    var i = (res + 2) + 2 * inx;
    mesh.addQuad( me, i, i+1, i+3, i+2 );
  }
  for ( var inx = 0; inx < res; ++ inx ) {
    var i = (res + 2) + 2 * inx;
    mesh.addTriangle( me, i+1, i+3, (res+1)*3+1 );
  }

  return me;
};


mesh.evaluateTrefoilKnot = function( s, t ) {
  var TwoPi = Math.PI * 2;
  var a = 0.5;
  var b = 0.3;
  var c = 0.5;
  var d = 0.1;
  var u = ( 1 - s ) * 2 * TwoPi;
  var v = t * TwoPi;
  var r = a + b * Math.cos( 1.5 * u );
  var x = r * Math.cos( u );
  var y = r * Math.sin( u );
  var z = c * Math.sin( 1.5 * u );

  var dv = vec3.create();
  dv[0] = -1.5 * b * Math.sin( 1.5 * u ) * Math.cos( u ) - ( a + b * Math.cos( 1.5 * u ) ) * Math.sin( u );
  dv[1] = -1.5 * b * Math.sin( 1.5 * u ) * Math.sin( u ) + ( a + b * Math.cos( 1.5 * u ) ) * Math.cos( u );
  dv[2] = 1.5 * c * Math.cos( 1.5 * u ); 

  var q = vec3.create( dv );
  vec3.normalize( q );
  var qvn = vec3.create( [ q[1], -q[0], 0 ] );
  vec3.normalize( qvn );  
  var ww = vec3.create();
  vec3.cross( q, qvn, ww );

  var range = vec3.create();
  range[0] = x + d * ( qvn[0] * Math.cos( v ) + ww[0] * Math.sin( v ) );
  range[1] = y + d * ( qvn[1] * Math.cos( v ) + ww[1] * Math.sin( v ) );
  range[2] = z + d * ww[2] * Math.sin( v );
    
  return range;
}

mesh.createTrefoilKnot = function( slices, stacks, drawType ) {
  var me = mesh.newMesh( mesh.SOLID, gl.TRIANGLES );

  var ds = 1.0 / slices;
  var dt = 1.0 / stacks;

  for ( var s = 0; s < 1 - ds / 2; s += ds) {
    for ( var t = 0; t < 1 - dt / 2; t += dt ) {
      var E = 0.01;
      
       var p = mesh.evaluateTrefoilKnot( s, t );
       var u = mesh.evaluateTrefoilKnot( s + E, t );
       u = vec3.subtract( u, p );
       var v = mesh.evaluateTrefoilKnot( s, t + E );
       v = vec3.subtract( v, p );
       var n = vec3.create();
       vec3.cross( u, v, n );
       vec3.normalize( n );

       mesh.addV( me, p[0], p[1], p[2], n[0], n[1], n[2], s * 18.0, t );
    }
  }
  var vertexCount = me.numArr;

  var n = 0;
  for ( var i = 0; i < slices; ++ i ) {
    for ( var j = 0; j < stacks; ++ j ) {
        
        mesh.addTriangle( me, n + j, n + (j + 1) % stacks, (n + j + stacks) % vertexCount );
        mesh.addTriangle( me, (n + j + stacks) % vertexCount, (n + (j + 1) % stacks) % vertexCount, (n + (j + 1) % stacks + stacks) % vertexCount );
     }
     n += stacks;
  }

  return me;
}


mesh.evaluateTorusKnot = function( s, t, p, q ) {
  var tube_rad = 0.2;
  var torus_rad = 0.25;

  /*
  var phi1 = ( s - 1 / ( q * Math.PI ) ) * Math.PI * 2;
  var r1 = Math.cos( q * phi1 ) + 2;
  var x1 = r1 * Math.cos( p * phi1 );
  var y1 = r1 * Math.sin( p * phi1 );
  var z1 = - Math.sin( q * phi1 );

  var phi2 = ( s + 1 / ( q * Math.PI ) ) * Math.PI * 2;
  var r2 = Math.cos( q * phi2 ) + 2;
  var x2 = r2 * Math.cos( p * phi2 );
  var y2 = r2 * Math.sin( p * phi2 );
  var z2 = - Math.sin( q * phi2 );

  var dir = vec3.create( [ x2 - x1, y2 - y1, z2 - z1 ] );
  vec3.normalize( dir );
  var dir_xy = vec3.create( [ dir[0], dir[1], 0 ] );
  vec3.normalize( dir_xy );
  var dir_xy_len = Math.sqrt( dir[0] * dir[0] + dir[1] + dir[1] );

  var phi = s * Math.PI * 2;
  var r = Math.cos( q * phi ) + 2;
  var x = r * Math.cos( p * phi );
  var y = r * Math.sin( p * phi );
  var z = - Math.sin( q * phi );
  
  var beta = t * Math.PI * 2;
  var tube_x = Math.sin( beta );
  var tube_y = Math.cos( beta );

  var vertex = vec3.create();
  vertex[0] = x + tube_rad * tube_x * -dir_xy[1] + tube_rad * tube_y * dir_xy_len * dir_xy[0]; 
  vertex[1] = y + tube_rad * tube_x * dir_xy[0] + tube_rad * tube_y * dir_xy_len * dir_xy[1]; 
  vertex[2] = z + tube_rad * tube_y * dir[2]; 
  */

  var phi = s * Math.PI * 2;
  var r = Math.cos( q * phi ) + 2;
  var dir_x = Math.cos( p * phi );
  var dir_y = Math.sin( p * phi );
  var x = r * dir_x;
  var y = r * dir_y;
  var z = - Math.sin( q * phi );
  
  var beta = t * Math.PI * 2;
  var tube_x = Math.sin( beta );
  var tube_y = Math.cos( beta );

  var dir_circle = vec3.create( [ -y, x, 0 ] );
  vec3.normalize( dir_circle );
  var dir_up = vec3.create( [ 0, Math.cos( q * phi ), 1 ] );
  vec3.normalize( dir_up );
  
  var vertex = vec3.create();
  vertex[0] = x + tube_rad * tube_x * dir_x + tube_rad * tube_y * dir_up[1] * dir_circle[0]; 
  vertex[1] = y + tube_rad * tube_x * dir_y + tube_rad * tube_y * dir_up[1] * dir_circle[1]; 
  vertex[2] = z + tube_rad * tube_y * dir_up[2]; 

  vertex[0] *= torus_rad;  
  vertex[1] *= torus_rad;
  vertex[2] *= torus_rad;
  return vertex;
}


mesh.createTorusKnot = function( slices, stacks, drawType, p_, q_ ) {
  var me = mesh.newMesh( mesh.SOLID, gl.TRIANGLES );

  if ( p_ == undefined )
    p_ = 3;
  if ( q_ == undefined )
    q_ = 7;
  
  var ds = 1.0 / slices;
  var dt = 1.0 / stacks;

  for ( var s = 0; s < 1 - ds / 2; s += ds) {
    for ( var t = 0; t < 1 - dt / 2; t += dt ) {
      var E = 0.001;
      
       var p = mesh.evaluateTorusKnot( s, t, p_, q_ );
       var u = mesh.evaluateTorusKnot( s - E, t, p_, q_ );
       u = vec3.subtract( u, p );
       var v = mesh.evaluateTorusKnot( s, t + E, p_, q_ );
       v = vec3.subtract( v, p );
       var n = vec3.create();
       vec3.cross( u, v, n );
       vec3.normalize( n );

       mesh.addV( me, p[0], p[1], p[2], n[0], n[1], n[2], s * 3.0 * p_ * q_, t );
    }
  }
  var vertexCount = me.numArr;

  var n = 0;
  for ( var i = 0; i < slices; ++ i ) {
    for ( var j = 0; j < stacks; ++ j ) {
        
        mesh.addTriangle( me, n + j, n + (j + 1) % stacks, (n + j + stacks) % vertexCount );
        mesh.addTriangle( me, (n + j + stacks) % vertexCount, (n + (j + 1) % stacks) % vertexCount, (n + (j + 1) % stacks + stacks) % vertexCount );
     }
     n += stacks;
  }

  return me;
}


mesh.createBlenderSkinMesh = function( blenderData, drawType ) {

  function isBitSet( value, position ) {
	  return value & ( 1 << position );
  }

  var me = mesh.newMesh( mesh.SOLID, gl.TRIANGLES );
  me.skin = 1;
  
  me.skinIndexArr = [];
  me.skinWeightArr = [];
  me.skinIndexArr.itemSize = 4;
  me.skinWeightArr.itemSize = 4;
  
  me.elementArr.itemSize = 3;
  me.elementKind = gl.TRIANGLES;
  me.numElemArr = 0;
  me.numArr = 0;
  
  var nUvLayers = 0;
	if ( blenderData.uvs !== undefined ) {
		// disregard empty arrays
		for ( i = 0; i < blenderData.uvs.length; i ++ ) {
			if ( blenderData.uvs[ i ].length ) nUvLayers ++;
		}
	}

  var materialIndex, uvIndex, normalIndex, colorIndex; 

  var influencesPerVertex = ( blenderData.influencesPerVertex !== undefined ) ? blenderData.influencesPerVertex : 2;
  var maxInx = 0;
  var f_i = 0;
  var len = blenderData.faces.length;
	while ( f_i < len ) {

    type = blenderData.faces[ f_i ++ ];
    
    isQuad              = isBitSet( type, 0 );
    hasMaterial         = isBitSet( type, 1 );
    hasFaceVertexUv     = isBitSet( type, 3 );
    hasFaceNormal       = isBitSet( type, 4 );
    hasFaceVertexNormal = isBitSet( type, 5 );
    hasFaceColor	      = isBitSet( type, 6 );
    hasFaceVertexColor  = isBitSet( type, 7 );

    // parse face indices
    var no_i = isQuad ? 4 : 3;
    var idxs = [0, 0, 0, 0];
    for ( var i_i = 0; i_i < no_i; ++ i_i ) {
      idxs[i_i] = blenderData.faces[ f_i ++ ];
      if ( idxs[i_i] > maxInx ) maxInx = idxs[i_i];
    }
    if ( maxInx >= blenderData.vertices.length / 3 ) {
      alert( "illegal element index" );
      break;
    }

    if ( no_i == 4 )
      me.elementArr.push( me.numArr, me.numArr+1, me.numArr+3, me.numArr+1, me.numArr+2, me.numArr+3 );
    else
      me.elementArr.push( me.numArr, me.numArr+1, me.numArr+2 );
    me.numArr += no_i;

    for ( var i_i = 0; i_i < no_i; ++ i_i ) {
      var x = blenderData.vertices[idxs[i_i]*3];
      var y = blenderData.vertices[idxs[i_i]*3+1];
      var z = blenderData.vertices[idxs[i_i]*3+2];
      me.vertexArr.push( x, y, z );
    }  

    // parse material
    if ( hasMaterial ) {
		  materialIndex = blenderData.faces[ f_i ++ ];
		}

    // parse texture coordinates
    if ( hasFaceVertexUv ) {
		  for ( i = 0; i < nUvLayers; i ++ ) {
			  var uvLayer = blenderData.uvs[ i ];
				for ( j = 0; j < no_i; j ++ ) {
				  var uvIndex = blenderData.faces[ f_i ++ ];
					var u = uvLayer[ uvIndex * 2 ];
					var v = uvLayer[ uvIndex * 2 + 1 ];
					me.textureArr.push( u, v );
				}
			}
		} else {
      for ( i = 0; i < nUvLayers; i ++ ) {
				for ( j = 0; j < no_i; j ++ ) {
          me.textureArr.push( 0, 0 );
				}
			}
    }

    // parse face normal vectors
    if ( hasFaceNormal ) {
		  normalIndex = blenderData.faces[ f_i ++ ];
			var nx = blenderData.normals[normalIndex*3];
      var ny = blenderData.normals[normalIndex*3+1];
      var nz = blenderData.normals[normalIndex*3+2];
      var nv = vec3.create( [ nx, ny, nz ] );
      vec3.normalize( nv );
      for ( i = 0; i < no_i; i ++ ) {
        me.normalArr.push( nv[0], nv[1], nv[2] );
      }
		}
				
    // parse vertex normal vectors    
    if ( hasFaceVertexNormal ) {
	    for ( i = 0; i < no_i; i ++ ) {
			  normalIndex = blenderData.faces[ f_i ++ ];
        var nx = blenderData.normals[normalIndex*3];
        var ny = blenderData.normals[normalIndex*3+1];
        var nz = blenderData.normals[normalIndex*3+2];
        var nv = vec3.create( [ nx, ny, nz ] );
        vec3.normalize( nv );
        me.normalArr.push( nv[0], nv[1], nv[2] );
      }
		}

    if ( hasFaceNormal == 0 && hasFaceVertexNormal == 0.0 ) {
      for ( i = 0; i < no_i; i ++ ) {
        me.normalArr.push( 0, 0, 1 );
      }
    }

    // parse face color
		if ( hasFaceColor ) {
		  colorIndex = blenderData.faces[ f_i ++ ];
			// hex = colors[ colorIndex ];
		}

    // parse vertex normal
		if ( hasFaceVertexColor ) {
		  for ( i = 0; i < no_i; i ++ ) {
			  colorIndex = blenderData.faces[ f_i ++ ];
				//hex = colors[ colorIndex ];
			}
		}

    // pars skin indizes and weights
    for ( var i = 0; i < no_i; ++ i ) {
    
      var a =                               blenderData.skinIndices[ idxs[i_i] * influencesPerVertex     ];
      var b = ( influencesPerVertex > 1 ) ? blenderData.skinIndices[ idxs[i_i] * influencesPerVertex + 1 ] : 0;
      var c = ( influencesPerVertex > 2 ) ? blenderData.skinIndices[ idxs[i_i] * influencesPerVertex + 2 ] : 0;
      var d = ( influencesPerVertex > 3 ) ? blenderData.skinIndices[ idxs[i_i] * influencesPerVertex + 3 ] : 0;
      me.skinIndexArr.push( a, b, c, d );
 
      var x =                               blenderData.skinWeights[ idxs[i_i] * influencesPerVertex     ];
      var y = ( influencesPerVertex > 1 ) ? blenderData.skinWeights[ idxs[i_i] * influencesPerVertex + 1 ] : 0;
      var z = ( influencesPerVertex > 2 ) ? blenderData.skinWeights[ idxs[i_i] * influencesPerVertex + 2 ] : 0;
      var w = ( influencesPerVertex > 3 ) ? blenderData.skinWeights[ idxs[i_i] * influencesPerVertex + 3 ] : 0;
      me.skinWeightArr.push( x, y, z, w );
    } 
  }
  me.numElemArr = me.elementArr.length;

  
  me.scale = blenderData.scale;
  me.bones = [];
  me.numOfBones = blenderData.bones.length;
  for ( var i = 0; i < me.numOfBones; ++ i ) {
    me.bones.push( new Object() );
    me.bones[i].parent = blenderData.bones[i].parent;
    me.bones[i].name = blenderData.bones[i].name;
    me.bones[i].pos = [ blenderData.bones[i].pos[0], blenderData.bones[i].pos[1], blenderData.bones[i].pos[2] ];
    me.bones[i].rotq = [ blenderData.bones[i].rotq[0], blenderData.bones[i].rotq[1], blenderData.bones[i].rotq[2], blenderData.bones[i].rotq[3] ];
    var mat4_pos = mat4.create();
    makeRotationFromQuaternion( mat4_pos, me.bones[i].rotq ); 
    mat4_pos[ 12 ] = me.bones[i].pos[0];
    mat4_pos[ 13 ] = me.bones[i].pos[1];
    mat4_pos[ 14 ] = me.bones[i].pos[2]; 
    me.bones[i].mat4_rel = mat4_pos;
    me.bones[i].mat4_abs = mat4.create( me.bones[i].mat4_rel );
  }

  for ( var i = 0; i < me.numOfBones; ++ i ) {
    var parent_i = blenderData.bones[i].parent;
    if ( parent_i < 0 || parent_i >= me.numOfBones || parent_i == i )
      continue;
    mat4.multiply(  me.bones[parent_i].mat4_abs, me.bones[i].mat4_rel, me.bones[i].mat4_abs ); 
  }

  return me;
}


//! /class timing
//!
//!
var timing = {};

timing.prevTimeAbs = 0;
timing.pause = 0;
timing.deltaTimeLastMs = 0;
timing.deltaTimeAbsMs = 0;

timing.pastTms = function() { return this.deltaTimeAbsMs; }

timing.init =  function() {
  this.prevTimeAbs = Date.now();
  this.pause = 0;
  this.deltaTimeLastMs = 0;
  this.deltaTimeAbsMs = 0;
};

timing.calcDeltaTimes = function() {
  var currentTimeAbs = Date.now();
  var delta = currentTimeAbs - this.prevTimeAbs;
  this.prevTimeAbs = currentTimeAbs;
  this.deltaTimeLastMs = this.pause == 0 ? delta : 0;
  this.deltaTimeAbsMs += this.deltaTimeLastMs;
};

timing.togglePause = function() {
  this.pause = this.pause != 0 ? 0 : 1;
}

//! \class motion
//!
//!
var motion = {};

motion.X = 0;
motion.Y = 1;
motion.Z = 2;
motion.U = 0;
motion.V = 1;
motion.R = 2;

motion.alternative = 0;

motion.angle = [ 0, 0, 0 ];
motion.angle.addon = [ 0, 0, 0 ];
motion.angle2 = [ 0, 0, 0 ];
motion.angle2.addon = [ 0, 0, 0 ];

motion.setup_rotation = function( u, v, r ) {
  motion.angle.addon[motion.U] = u;
  motion.angle.addon[motion.V] = v;
  motion.angle.addon[motion.R] = r;
}

motion.init_rotation2 = function( u, v, r ) {
  motion.angle2 = [ u, v, r ];
  motion.angle2.addon = [ 0.0, 0.0, 0.0 ];
}

motion.angle.prop = function( axis, angle ) {

  if ( angle ) this[axis] = angle;
  return this[axis] + this.addon[axis];
}
motion.angle.u = function( angle ) { return this.prop( motion.U, angle ); }
motion.angle.v = function( angle ) { return this.prop( motion.V, angle ); }
motion.angle.r = function( angle ) { return this.prop( motion.R, angle ); }
motion.angle.per_s = function( axis, sec ) { return this.prop( axis, 2.0 * Math.PI / sec * timing.deltaTimeAbsMs / 1000.0 ); }

motion.do = function( mat4_motion ) {
  mat4.rotateZ( mat4_motion, ( this.angle[this.R] + this.angle.addon[this.R] ) % ( 2.0 * Math.PI ), mat4_motion );
  mat4.rotateY( mat4_motion, ( this.angle[this.V] + this.angle.addon[this.V] ) % ( 2.0 * Math.PI ), mat4_motion );
  mat4.rotateX( mat4_motion, ( this.angle[this.U] + this.angle.addon[this.U] ) % ( 2.0 * Math.PI ), mat4_motion );
  return mat4_motion;
}

//! /class hid_events
//!
//!
var hid_events = {};

hid_events.PIXELS_PER_ROUND = 1800;
hid_events.userRotateSpeed = 1.0;

hid_events.MOUSE_BTN_L = 0;
hid_events.MOUSE_BTN_M = 1;
hid_events.MOUSE_BTN_R = 2;

hid_events.MOUSE_NON = 0;
hid_events.MOUSE_ROTATE = 1;

hid_events.mouse_state = hid_events.MOUSE_NON;
hid_events.mouse_x = 0;
hid_events.mouse_y = 0;
hid_events.pause_bk = 0;

hid_events.onKeyDown = function( event ) {
  // ...
}
hid_events.onKeyUp = function( event ) {
  // ...
}
hid_events.init_key = function() {
  //window.addEventListener( 'keydown', onKeyDown, false );
	//window.addEventListener( 'keyup', onKeyUp, false );
}

hid_events.onMouseMove = function( event ) {
  if ( hid_events.mouse_state == hid_events.MOUSE_NON ) return;
  event.preventDefault();

  var delta_x = event.clientX - hid_events.mouse_x;
  var delta_y = event.clientY - hid_events.mouse_y;
  hid_events.mouse_x = event.clientX;
  hid_events.mouse_y = event.clientY;

  var rotAngleU = 2 * Math.PI * delta_y / hid_events.PIXELS_PER_ROUND * hid_events.userRotateSpeed;
	var rotAngleV = 2 * Math.PI * delta_x / hid_events.PIXELS_PER_ROUND * hid_events.userRotateSpeed;
	
  if ( motion.alternative == 0 ) {
    motion.angle.addon[motion.U] += rotAngleU;
    motion.angle.addon[motion.V] += rotAngleV;
  } else {
    motion.angle2.addon[motion.U] += rotAngleU;
    motion.angle2.addon[motion.V] += rotAngleV;
  }

  // ...
}
hid_events.onMouseDown = function( event ) {
  if ( hid_events.mouse_state != hid_events.MOUSE_NON ) return;

  var rect = gl.canvas.getBoundingClientRect();
  if ( event.clientX < rect.left || event.clientX > rect.right ) return;
  if ( event.clientY < rect.top || event.clientY > rect.bottom ) return;
  //if ( event.offsetX < 0 || event.offsetX > rect.width ) return;
  //if ( event.offsetY < 0 || event.offsetY > rect.height ) return;

  event.preventDefault();
  
  if ( event.button == hid_events.MOUSE_BTN_R )
  {
    timing.togglePause();
    return;
  }
 
  hid_events.pause_bk = timing.pause;
  timing.pause = 1;
  hid_events.mouse_state = hid_events.MOUSE_ROTATE;
  hid_events.mouse_x = event.clientX;
  hid_events.mouse_y = event.clientY;
  
  //document.addEventListener( 'mousemove', this.onMouseMove, false );
	//document.addEventListener( 'mouseup', this.onMouseUp, false );
}
hid_events.onMouseUp = function( event ) {
  if ( hid_events.mouse_state == hid_events.MOUSE_NON ) return;
hid_events.mouse_state = hid_events.MOUSE_NON;
  timing.pause = hid_events.pause_bk;
  //document.removeEventListener( 'mousemove', this.onMouseMove, false );
	//document.removeEventListener( 'mouseup', this.onMouseUp, false );
}
hid_events.onMouseWheel = function( event ) {
  // ...
}
hid_events.init_mouse = function( domElement ) {
  this.domElement = ( domElement !== undefined ) ? domElement : document;

  this.domElement.addEventListener( 'contextmenu', function ( event ) { event.preventDefault(); }, false );
	this.domElement.addEventListener( 'mousedown', hid_events.onMouseDown, false );
	this.domElement.addEventListener( 'mousemove', hid_events.onMouseMove, false );
	this.domElement.addEventListener( 'mouseup', hid_events.onMouseUp, false );
	this.domElement.addEventListener( 'mousewheel', hid_events.onMouseWheel, false );
	this.domElement.addEventListener( 'DOMMouseScroll', hid_events.onMouseWheel, false ); // firefox
}

var doc_data_exchange = {

create_val : function( domElement, doc_name, val, scale, doc_text  ) {

  if ( doc_name == undefined )
    return;
  var elem = new Object();
  
  elem.domElement = domElement ? domElement : document;
  elem.doc_name = doc_name;
  elem.doc_text = doc_text != undefined ? doc_text : "";
  elem.scale = scale != undefined ? scale : 1.0; 
  if ( val != undefined )
    elem.val = val;
  if ( val != undefined )
    elem.init_val = val;  

  elem.get = function () {
    this.val = this.domElement.getElementById( this.doc_name ).value / this.scale;
    if ( this.mapFrom != undefined )
      this.val = this.mapFrom( this.val );
    this.view();
    return this.val;
  }

  elem.set = function ( val ) {
    if ( val != undefined ) {
      this.val = val;
    }
    if ( this.mapTo != undefined )
      this.val = this.mapTo( this.val );
    this.domElement.getElementById( this.doc_name ).value = this.val * this.scale;
    this.view();
    return this;
  }

  elem.view = function() {
    if ( this.onView != undefined ) {
      this.onView();
    }
    if ( this.doc_text != undefined && this.doc_text != "" ) {
      var val = this.fix != undefined ? this.val.toFixed( this.fix ) : this.val;
      document.getElementById( this.doc_text ).innerHTML = val;
    }
  }

  return elem;
},

create_check : function( domElement, doc_name, val  ) {

  if ( doc_name == undefined )
    return;
  var elem = new Object();
  
  elem.domElement = domElement != undefined ? domElement : document;
  elem.doc_name = doc_name;
  if ( val != undefined )
    elem.val = val;
  if ( val != undefined )
    elem.init_val = val; 

  elem.get = function () {
    this.val = this.domElement.getElementById( this.doc_name ).checked;
    return this.val;
  }

  elem.set = function ( val ) {
    if ( val != undefined ) {
      this.val = val;
    }
    this.domElement.getElementById( this.doc_name ).checked = this.val != 0 ? 1 : 0;
    return this;
  }

  return elem;
}

};

//! \class glsl
//!
//!
var glsl = {};

// standard vertex shader
glsl.std_vert =
"attribute vec3 aVertPos; \n" +
"attribute vec3 aNormalVec; \n" +
"attribute vec2 aTexCoord; \n" +

"varying vec4 vPosEs; \n" +
"varying vec3 vNvEs; \n" +
"varying vec2 vTexCoord; \n" +

"uniform mat4 u_matTexture; \n" +
"uniform mat4 u_matModelView; \n" +
"uniform mat3 u_matNormal; \n" +
"uniform mat4 u_matProjection; \n" +

"void main( void ) \n" +
"{ \n" +
"    vPosEs = u_matModelView * vec4( aVertPos.xyz, 1.0 ); \n" +
"    vNvEs = u_matNormal * normalize( aNormalVec ); \n" +
"    vec4 tempTex = u_matTexture * vec4( aTexCoord.xy, 0.0, 1.0 ); \n" +
"    vTexCoord = tempTex.xy; \n" +
"    gl_Position = u_matProjection * vPosEs; \n" +
"} \n";


// standard screenspace vertex shader
glsl.std_screenspace_vert =
"attribute vec3 aVertPos; \n" +
"attribute vec3 aNormalVec; \n" +
"attribute vec2 aTexCoord; \n" +
  
"varying vec4 vPosEs; \n" +
"varying vec3 vNvEs; \n" +
"varying vec2 vTexCoord; \n" +

"uniform mat4 u_matTexture; \n" +
  
"void main( void ) \n" +
"{ \n" +
"    vPosEs = vec4( aVertPos.xyz, 1.0 ); \n" +
"    vNvEs = aNormalVec; \n" +
"    vec4 tempTex = u_matTexture * vec4( aTexCoord.xy, 0.0, 1.0 ); \n" +
"    vTexCoord = tempTex.xy; \n" +
"    gl_Position = vPosEs; \n" +
"} \n";


// standard 2D vertex shader
glsl.std_2D_vert =
"attribute vec3 aVertPos; \n" +
"attribute vec3 aNormalVec; \n" +
"attribute vec2 aTexCoord; \n" +
  
"varying vec4 vPosEs; \n" +
"varying vec3 vNvEs; \n" +
"varying vec2 vTexCoord; \n" +

"uniform mat4 u_matTexture; \n" +
"uniform mat4 u_matModelView; \n" +
"uniform mat3 u_matNormal; \n" +
  
"void main( void ) \n" +
"{ \n" +
"    vPosEs = u_matModelView * vec4( aVertPos.xyz, 1.0 ); \n" +
"    vNvEs = u_matNormal * normalize( aNormalVec ); \n" +
"    vec4 tempTex = u_matTexture * vec4( aTexCoord.xy, 0.0, 1.0 ); \n" +
"    vTexCoord = tempTex.xy; \n" +
"    gl_Position = vPosEs; \n" +
"} \n";


// standard vertex shader
glsl.skinning_vert =
"attribute vec3 aVertPos; \n" +
"attribute vec3 aNormalVec; \n" +
"attribute vec2 aTexCoord; \n" +
"attribute vec4 aSkinIndex; \n" +
"attribute vec4 aSkinWeight; \n" +

"varying vec4 vPosEs; \n" +
"varying vec3 vNvEs; \n" +
"varying vec2 vTexCoord; \n" +

"uniform mat4 u_matModelView; \n" +
"uniform mat4 u_matProjection; \n" +
"uniform mat3 u_matNormal; \n" +

"uniform mat4 u_matBind; \n" +
"uniform mat4 u_matBindInverse; \n" +

"#define MAX_BONES 30 \n" +

"#ifdef BONE_TEXTURE \n" +

"uniform sampler2D boneTexture; \n" +
"uniform int u_boneTextureWidth; \n" +
"uniform int u_boneTextureHeight; \n" +

"mat4 getBoneMatrix( const in float i ) \n" +
"{ \n" +
"  float j = i * 4.0; \n" +
"  float x = mod( j, float( u_boneTextureWidth ) ); \n" +
"  float y = floor( j / float( u_boneTextureWidth ) ); \n" +
  
"  float dx = 1.0 / float( u_boneTextureWidth ); \n" +
"  float dy = 1.0 / float( u_boneTextureHeight ); \n" +
"  y = dy * ( y + 0.5 ); \n" +
  
"  vec4 v1 = texture2D( u_boneTexture, vec2( dx * ( x + 0.5 ), y ) ); \n" +
"  vec4 v2 = texture2D( u_boneTexture, vec2( dx * ( x + 1.5 ), y ) ); \n" +
"  vec4 v3 = texture2D( u_boneTexture, vec2( dx * ( x + 2.5 ), y ) ); \n" +
"  vec4 v4 = texture2D( u_boneTexture, vec2( dx * ( x + 3.5 ), y ) ); \n" +
  
"  mat4 bone = mat4( v1, v2, v3, v4 ); \n" +
"  return bone; \n" +
"} \n" +
    
"#else \n" +

"uniform mat4 u_boneGlobalMatrices[ MAX_BONES ]; \n" +

"mat4 getBoneMatrix( const in float i ) \n" +
"{ \n" +
"  mat4 bone = u_boneGlobalMatrices[ int(i) ]; \n" +
"  return bone; \n" +
"} \n" +

"#endif \n" +

"void main( void ) \n" +
"{ \n" +
"  mat4 boneMatX = getBoneMatrix( aSkinIndex.x ); \n" +
"  mat4 boneMatY = getBoneMatrix( aSkinIndex.y ); \n" +
"  mat4 boneMatZ = getBoneMatrix( aSkinIndex.z ); \n" +
"  mat4 boneMatW = getBoneMatrix( aSkinIndex.w ); \n" +

"  vec4 skinVertex = u_matBind * vec4( aVertPos, 1.0 ); \n" +

"  vec4 skinned = vec4( 0.0 ); \n" +
"  skinned += boneMatX * skinVertex * aSkinWeight.x; \n" +
"  skinned += boneMatY * skinVertex * aSkinWeight.y; \n" +
"  skinned += boneMatZ * skinVertex * aSkinWeight.z; \n" +
"  skinned += boneMatW * skinVertex * aSkinWeight.w; \n" +
"  skinned  = u_matBindInverse * skinned; \n" +

"  mat4 skinMatrix = mat4( 0.0 ); \n" +
"	 skinMatrix += aSkinWeight.x * boneMatX; \n" +
"  skinMatrix += aSkinWeight.y * boneMatY; \n" +
"  skinMatrix += aSkinWeight.z * boneMatZ; \n" +
"  skinMatrix += aSkinWeight.w * boneMatW; \n" +
"  skinMatrix  = u_matBindInverse * skinMatrix * u_matBind; \n" +

"  vec4 skinnedNormal = skinMatrix * vec4( aNormalVec, 0.0 ); \n" +
"  vec3 objectNormal = skinnedNormal.xyz; \n" +
"  vec3 transformedNormal = u_matNormal * objectNormal; \n" +

"  vec4 mvPosition = u_matModelView * skinned; \n" +
"  gl_Position = u_matProjection * mvPosition; \n" +

"  vNvEs = normalize( transformedNormal ); \n" +
"  vPosEs = vec4( mvPosition.xyz / mvPosition.w, 1.0 ); \n" +
"  vTexCoord = aTexCoord.st; \n" +
"} \n";

glsl.vertex_shader = {};
glsl.vertex_shader.std = {
  type : "vert",
  code : glsl.std_vert
};
glsl.vertex_shader.screenspace = {
  type : "vert",
  code : glsl.std_screenspace_vert
};
glsl.vertex_shader.std_2D = {
  type : "vert",
  code : glsl.std_2D_vert
};
glsl.vertex_shader.skinning = {
  type : "vert",
  code : glsl.skinning_vert
};

glsl.vertex_shader.link_uniforms = function( prog ) {

  prog.mat4_std_tex = mat4.identity();
  prog.mat4_std_tex_flip_y = mat4.identity();
  mat4.scale( prog.mat4_std_tex_flip_y, [1.0, -1.0, 1.0], prog.mat4_std_tex_flip_y );

  // link attribute loactions
  prog.aVertPos = gl_util.linkBufferLocation( prog, "aVertPos" );
  prog.aNormalVec = gl_util.linkBufferLocation( prog, "aNormalVec" );
  prog.aTexCoord = gl_util.linkBufferLocation( prog, "aTexCoord" );
  prog.aSkinIndex = gl_util.linkBufferLocation( prog, "aSkinIndex" );
  prog.aSkinWeight = gl_util.linkBufferLocation( prog, "aSkinWeight" );

  // link matrix uniform loactions
  prog.u_matTexture     = gl.getUniformLocation( prog, "u_matTexture" );
  prog.u_matModelView   = gl.getUniformLocation( prog, "u_matModelView" );
  prog.u_matNormal      = gl.getUniformLocation( prog, "u_matNormal" );
  prog.u_matProjection  = gl.getUniformLocation( prog, "u_matProjection" );

  // link common uniform locations
  prog.u_time_ms        = gl.getUniformLocation( prog, "u_time_ms" );
  prog.u_view_port_size = gl.getUniformLocation( prog, "u_view_port_size" );
}


// tangent space
glsl.tangent_space =
"mat3 TBN( vec3 ptV, vec3 vecN, vec2 texCoord ) \n" +
"{ \n" +
"  vec3 N = vecN * ( vecN.z > 0.0 ? 1.0 : -1.0 ); \n" +
"  vec3  p_dx   = dFdx( ptV ); \n" +
"  vec3  p_dy   = dFdy( ptV ); \n" +
"  vec2  tc_dx  = dFdx( texCoord ); \n" +
"  vec2  tc_dy  = dFdy( texCoord ); \n" +
"  vec3 dp1perp = cross( N, p_dx ); \n" +
"  vec3 dp2perp = cross( p_dy, N ); \n" +
"  vec3 T = dp2perp * tc_dx.x + dp1perp * tc_dy.x; \n" +
"  vec3 B = dp2perp * tc_dx.y + dp1perp * tc_dy.y; \n" +
"  return mat3( normalize( T ), normalize( B ), N ); \n" +
"} \n" +

"mat3 inverseTBN( vec3 ptV, vec3 vecN, vec2 texCoord ) \n" +
"{ \n" +
"  vec3 N = vecN * ( vecN.z > 0.0 ? 1.0 : -1.0 ); \n" +
"  vec3  p_dx   = dFdx( ptV ); \n" +
"  vec3  p_dy   = dFdy( ptV ); \n" +
"  vec2  tc_dx  = dFdx( texCoord ); \n" +
"  vec2  tc_dy  = dFdy( texCoord ); \n" +
"  vec3 dp1perp = cross( N, p_dx ); \n" +
"  vec3 dp2perp = cross( p_dy, N ); \n" +
"  vec3 T = dp2perp * tc_dx.x + dp1perp * tc_dy.x; \n" +
"  vec3 B = dp2perp * tc_dx.y + dp1perp * tc_dy.y; \n" +
"  float invmax = inversesqrt( max( dot( T, T ), dot( B, B ) ) ); \n" +
"  return mat3( normalize( T * invmax ), normalize( B * invmax ), N ); \n" +
"} \n" +

"mat3 mat3_inverse( mat3 A ) \n" +
"{ \n" +
"  //float a = A[0][0]; float b = A[0][1]; float c = A[0][2]; \n" +
"  //float d = A[1][0]; float e = A[1][1]; float f = A[1][2]; \n" +
"  //float g = A[2][0]; float h = A[2][1]; float i = A[2][2]; \n" +
"  //return mat3( \n" +
"  //  vec3( e*i-f*h, c*h-b*i, b*f-c*e ), \n" + 
"  //  vec3( f*g-d*i, a*i-c*g, c*d-a*f ), \n" +
"  //  vec3( d*h-e*g, b*g-a*h, a*e-b*d ) ); \n" +
"  mat3 M_t = mat3( \n" +
"    vec3( A[0][0], A[1][0], A[2][0] ), \n" +
"    vec3( A[0][1], A[1][1], A[2][1] ), \n" +
"    vec3( A[0][2], A[1][2], A[2][2] ) ); \n" +
"  float det = dot( cross( M_t[0], M_t[1] ), M_t[2] ); \n" +
"  mat3 adjugate = mat3( cross( M_t[1], M_t[2] ), \n" +
"                        cross( M_t[2], M_t[0] ), \n" +
"                        cross( M_t[0], M_t[1] ) ); \n" +
"  return adjugate / det; \n" +
"} \n";

// fragment shader standard head
glsl.fragment_shader = {};

// fragmant standard head
glsl.fragment_shader.std_head =
//#extension GL_EXT_shader_texture_lod : enable
"#extension GL_OES_standard_derivatives : enable \n" +
//#extension GL_OES_texture_float : enable

"precision mediump float; \n" +

"varying vec4 vPosEs; \n" +
"varying vec3 vNvEs; \n" +
"varying vec2 vTexCoord; \n";

// SSAO vertex shader 
glsl.SSAO_kernelSize = gl_util.isIE() ? 32 : 64;
glsl.SSAO_shader =
"uniform sampler2D u_colorSampler; \n" +
"uniform sampler2D u_depthSampler; \n" +
"uniform sampler2D u_nvSampler; \n" +
"uniform sampler2D u_posSampler; \n" +
"uniform sampler2D u_ssaoKernelSampler; \n" +
"uniform sampler2D u_ssaoNoiseSampler; \n" +

"uniform mat4  u_matProjection; \n" +
"uniform vec3  u_depthRange; \n" +
"uniform vec2  u_view_port_size; \n" +
"uniform float u_ssao_radius; \n" +
"uniform int   u_weight_mode; \n" +
"uniform int   u_ssaoParamKernelSize; \n" +
"uniform int   u_ssaoParamNoiseSize; \n" +

"float DEPTH_TO_LINEAR_Z_NF( float D, float N, float F ) { return (2.0 * N) / (F + N - ( D * (F - N))); } \n" +
"float DEPTH_TO_LINEAR_Z( float D ) { return DEPTH_TO_LINEAR_Z_NF( D, u_depthRange.x, u_depthRange.y ); } \n" +

"float Depth( in sampler2D depthSampler, in vec2 texC ) \n" +
"{ \n" +
"  vec3 depthVal = texture2D( depthSampler, texC.st ).xyz; \n" + 
"  return depthVal.x * 256.0 + depthVal.y + depthVal.z / 256.0; \n" +
"} \n" +

"vec3 NV( in sampler2D nvSampler, in vec2 texC ) \n" +
"{ \n" +
"  vec3 nvVal = texture2D( nvSampler, texC.st ).xyz; \n" + 
"  return normalize( nvVal * 2.0 - 1.0 ); \n" +
"} \n" +

"vec3 Pos( in sampler2D posSampler, in vec2 texC ) \n" +
"{ \n" +
"  vec4 posVal = texture2D( posSampler, texC.st ); \n" +  
"  return ( posVal.xyz * 2.0 - 1.0 ) / posVal.w; \n" +
"} \n" +

"void main(void) \n" +
"{ \n" +
"  vec2 objTexCoord = vTexCoord; \n" +
"  vec4 color = texture2D( u_colorSampler, vTexCoord ); \n" +

"  float linearZ = 0.0; \n" +
"  float luminosity = 0.0; \n" +
"  float occlusion  = 0.0; \n" +
"  if ( color.a > 0.0 ) \n" +
"  { \n" +
"    float radius      = u_ssao_radius; \n" +
"    vec3 fragPos      = Pos( u_posSampler, objTexCoord.st ); \n" +
"    vec3 fragNV       = NV( u_nvSampler, objTexCoord.st ); \n" +
"    float fragDepth   = Depth( u_depthSampler, objTexCoord.st ); \n" +
"    vec2 texelSize    = 1.0 / u_view_port_size.xy; \n" +
"    vec2 noiseScale   = u_view_port_size.xy / float( u_ssaoParamNoiseSize ); \n" +
"    vec4 randomVal    = texture2D( u_ssaoNoiseSampler, vec2( objTexCoord.s * noiseScale.x - floor( objTexCoord.s * noiseScale.x ), objTexCoord.t * noiseScale.y - floor( objTexCoord.t * noiseScale.y ) ) ); \n" +
"    vec3 randomVec    = randomVal.xyz * 2.0 - 1.0; \n" +
"    vec3 tangent      = normalize( randomVec - fragNV * dot( randomVec, fragNV ) ); \n" +
"    mat3 TBN          = mat3( tangent, cross( fragNV, tangent ), fragNV ); \n" +

//vec4 fragPosProj  = u_matProjection * vec4( fragPos.xyz, 1.0 );
//vec4 offsetProj   = u_matProjection * vec4( fragPos.xy + radius * 0.707, fragPos.z, 1.0 );

//vec2 texCoordProj = 0.5 + 0.5 * fragPosProj.xy / fragPosProj.w;
"    vec2 texCoordProj = objTexCoord; \n" +

// * 0.5 : transform to texture coordinate range [-1.0, 1.0] -> [0.0, 1.0]
//vec2 texScaleProj = 0.5 * ( offsetProj.xy / offsetProj.w - fragPosProj.xy / fragPosProj.w ); 
// optimization : Skip addition and subtraction of the asymmetric part of the projection matrix (Multiply only with the scaling).
"    vec2 texScaleProj = 0.5 * radius * 0.707 * vec2( abs( u_matProjection[0][0] / fragPos.z ), abs( u_matProjection[1][1] / fragPos.z ) ); \n" +
"    float scaleZ = 0.5 * abs( u_matProjection[2][3] / ( radius - fragPos.z ) + u_matProjection[2][3] / fragPos.z ); \n" +

// optimization : No more need of 'fragPos' and 'u_posSampler'
//float fragZ = fragDepth * 2.0 - 1.0; // [0.0, 1.0] -> [-1.0, 1.0]
//fragZ = DEPTH_TO_LINEAR_Z( fragZ ); // [-1.0, 1.0] -> Linear depth, in world units
//fragZ = u_matProjection[2][3] / ( fragZ - u_matProjection[2][2] ); // [-1.0, 1.0] -> Linear depth, in world units
//vec2 texScaleProj = 0.5 * radius * 0.707 * vec2( abs( u_matProjection[0][0] / fragZ ), abs( u_matProjection[1][1] / fragZ ) );
    
"    float kernelPtCount = 0.0; \n" +
"    for ( int inx = 0; inx < " + glsl.SSAO_kernelSize.toString() + "; ++ inx ) \n" +
"    { \n" +
"      int inx2 = inx;// * 2; \n" +
"      if ( inx2 >= u_ssaoParamKernelSize ) \n" +
"        break; \n" +
"      vec4 sampleVal = texture2D( u_ssaoKernelSampler, vec2( float( u_ssaoParamKernelSize - inx2 - 1 ) / float( u_ssaoParamKernelSize ), 0.0 ) ); \n" +
"      vec3 sampleVec = ( ( sampleVal.xyz * 2.0 - 1.0 ) * sampleVal.w ); \n" +
"      vec3 sampleRel = TBN * sampleVec; \n" +

"      vec2 sampleTexC = texCoordProj.xy + sampleRel.xy * texScaleProj.xy; \n" +
"      vec2 rangeTest = step( vec2( 0.0, 0.0 ), sampleTexC ) * step( sampleTexC, vec2( 1.0, 1.0 ) ); \n" +
"      float sampleDepth = Depth( u_depthSampler, sampleTexC.st ); \n" +
"      float w = rangeTest.x * rangeTest.y; \n" +
"      kernelPtCount += w; \n" +
//occlusion += w * ( 1.0 - step( sampleDepth - fragDepth + sampleRel.z * scaleZ, 0.0 ) );
"      if ( u_weight_mode == 0 ) // flat \n" +
"        occlusion += w * ( 1.0 - step( sampleDepth - fragDepth, 0.0 ) ); \n" +
"      else if ( u_weight_mode == 1 ) // smooth step \n" +
"      { \n" +
"        //float sampleDelta = w * ( sampleDepth - fragDepth + sampleRel.z * scaleZ ); \n" +
"        float sampleDelta = sampleDepth - fragDepth; \n" +
"        occlusion += w * ( 1.0 - step( sampleDelta, 0.0 ) ) * ( sampleDelta < scaleZ ? 1.0 : 0.0 ); \n" +
"        //occlusion += ( sampleDelta > 0.0 ) ? smoothstep( 0.0, 1.0, scaleZ / sampleDelta ) : 0.0; \n" +
"      } \n" +
"      else if ( u_weight_mode == 2 ) // experimental \n" +
"      { \n" +
"        vec4 samplePos = vec4( fragPos.xyz + sampleRel * radius, 1.0 ); \n" +
"        samplePos.xyz = samplePos.xyz / samplePos.w; \n" +
"        vec3 sampleFragPos = Pos( u_posSampler,  sampleTexC.st ); \n" +
"        occlusion += w * ( 1.0 - step( samplePos.z - sampleFragPos.z, 0.0 ) ); \n" +
"      } \n" +
"      else // if ( u_weight_mode == 3 ) // experimental range check \n" +
"      { \n" +
"        vec4 samplePos = vec4( fragPos.xyz + sampleRel * radius, 1.0 ); \n" +
"        samplePos.xyz = samplePos.xyz / samplePos.w; \n" +
"        vec3 sampleFragPos = Pos( u_posSampler,  sampleTexC.st ); \n" +
"        float deltaZ = samplePos.z - sampleFragPos.z; \n" +
"        occlusion += w * ( deltaZ < radius ? ( 1.0 - step( deltaZ, 0.0 ) ) : 1.0 ); \n" +
"      } \n" +

// optimization : Use half the kernel size and test the sample point and the opposite sample point at once.
// Probably this optimization improves the distribution of the sample points. 
/*
sampleTexC = texCoordProj.xy - sampleRel.xy * texScaleProj.xy;
rangeTest = step( vec2( 0.0, 0.0 ), sampleTexC ) * step( sampleTexC, vec2( 1.0, 1.0 ) );
sampleDepth = Depth( u_depthSampler, sampleTexC.st );
w = rangeTest.x * rangeTest.y * ( 1.0 - step( sampleDepth, 0.0 ) );
kernelPtCount += w;
float sampleDelta = w * ( sampleDepth - fragDepth );
occlusion += ( sampleDelta > 0.0 ) ? smoothstep( 0.0, 1.0, radius / sampleDelta ) : 0.0;  
*/

// optimization : do it reverse and and break if texels size is below the threshold
//  It is not only an optimization, becaus the multiple inclusion of the origin samples distorts the result.
"      if ( abs( sampleVec.x * texScaleProj.x ) < texelSize.x * 0.5 && abs( sampleVec.y * texScaleProj.y ) < texelSize.y * 0.5 ) \n" +
"        break;  \n" +
"    } \n" +
"    occlusion = occlusion / kernelPtCount; \n" +
"    linearZ = 1.0 - fragDepth; \n" +
"    luminosity = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b; \n" + 
"  } \n" +

"  gl_FragColor = vec4( occlusion, linearZ, luminosity, 1.0 ); \n" +
"} \n";

glsl.fragment_shader.SSAO = {
  type : "frag",  
  code : [
    glsl.fragment_shader.std_head,
    glsl.SSAO_shader
  ]
};

// fragmant standard common uniforms
glsl.fragment_shader.common_uniforms =
"uniform float u_time_ms; \n" +
"uniform vec2 u_view_port_size; \n";

// fragmant standard 2D drawing functions"
glsl.fragment_shader.draw2D_functions =
"vec4 PixelSizeInfo( in vec2 vpSize ) \n" +
"{ \n" +
"  vec4 sizeInfo; \n" +
"  sizeInfo.xy = 2.0 / vpSize; \n" +
"  sizeInfo.z  = vpSize.y / vpSize.x; \n" +
"  sizeInfo.w  = length( sizeInfo.xy ); \n" +
"  return sizeInfo; \n" + 
"} \n" +

"float Intensity( in vec2 dist, in vec4 pixel_info ) \n" +
"{ \n" +
"  float ratio = pixel_info.z; \n" +
"  vec2  size  = pixel_info.xy; \n" +
"  return ( 1.0 - smoothstep( 0.0, size.x / ratio, abs( dist.x ) ) ) * ( 1.0 - smoothstep( 0.0, size.y, abs( dist.y ) ) ); \n" +
"} \n" +

"float IntensityOr( in vec2 dist, in vec4 pixel_info ) \n" +
"{ \n" +
"  float ratio = pixel_info.z; \n" +
"  vec2  size  = pixel_info.xy; \n" +
"  return max( 1.0 - smoothstep( 0.0, size.x / ratio, abs( dist.x ) ), 1.0 - smoothstep( 0.0, size.y, abs( dist.y ) ) ); \n" +
"} \n" +

"vec4 MixColor( in vec4 baseCol, in vec4 addCol, float intensity ) \n" +
"{ \n" +
"  return mix( baseCol, addCol, intensity ); \n" +
"} \n" +

"vec4 AddCircle( in vec2 pos, in vec2 cpt, float rad, in vec4 pixel_info, in vec4 baseCol, in vec4 addCol ) \n" +
"{ \n" +
"  vec2 circle_pos = pos - cpt; \n" +
"  vec2 circle_dist = circle_pos - normalize( circle_pos ) * rad; \n" +
"  float circle_intensity = Intensity( circle_dist, pixel_info ); \n" +
"  return MixColor( baseCol, addCol, circle_intensity ); \n" +
"} \n" +

"vec4 AddLine( in vec2 pos, in vec2 p1, in vec2 p2, in vec4 pixel_info, in vec4 baseCol, in vec4 addCol ) \n" +
"{ \n" +
"  float len_pixel = pixel_info.w; \n" +
"  vec2 vec_line = p2 - p1; \n" +
"  float len_line = length( vec_line ); \n" +
"  vec2 dir_line = normalize( vec_line ); \n" +
"  vec2 vec_pos  = pos - p1; \n" +
"  float cosXpt = dot( dir_line, vec_pos ); \n" +
"  vec2 xpt = p1 + dir_line * cosXpt; \n" +
"  vec2 dist = pos - xpt; \n" +
"  float intensity = Intensity( dist, pixel_info ) * smoothstep( -len_pixel, 0.0, cosXpt ) * ( 1.0 - smoothstep( len_line, len_line + len_pixel, cosXpt ) ); \n" +
"  return MixColor( baseCol, addCol, intensity ); \n" +
"} \n" +

"vec4 AddGrid( in vec2 vpSize, in vec2 pos, in vec2 scale, in vec2 offs, in vec2 grid, in vec4 pixel_info, in vec4 baseCol, in vec4 addCol ) \n" +
"{ \n" +
"  vec2 posRel = pos * scale + offs; \n" +
"  vec2 delta = abs( posRel / grid ); \n" +
"  delta = ( delta - floor(delta) ) * grid / scale;" +
"  float intensity = IntensityOr( delta, pixel_info ); \n" +
"  return MixColor( baseCol, addCol, intensity ); \n" +
"} \n";

// fragmant standard 2D drawing functions"
glsl.fragment_shader.PlotFunction = function( postfix, func_expr ) {
  
  var codeStr =

  "float f" + postfix.toString() + "( in float x ) \n" +
  "{ \n" +
  "  float y = " + func_expr + "; \n" +
  "  return y; \n" +
  "} \n" +

  "float IntensityFunc" + postfix.toString() + "( in vec2 vpSize, in vec2 pos, in vec2 scale, in vec2 offs ) \n" +
  "{ \n" +
  "  vec2 pixelSize = 1.0 / vpSize; \n" +
  "  vec3 h; \n" +
  "  h.x = ( f" + postfix.toString() + "( offs.x + scale.x * ( pos.x - pixelSize.x ) ) - offs.y ) / scale.y; \n" +
  "  h.y = ( f" + postfix.toString() + "( offs.x + scale.x * pos.x ) - offs.y ) / scale.y; \n" +
  "  h.z = ( f" + postfix.toString() + "( offs.x + scale.x * ( pos.x + pixelSize.x ) ) - offs.y ) / scale.y; \n" +
  "  float intensity = 0.0; \n" +
  "  if ( pos.y <= h.y ) \n" +
  "  { \n" +
  "    if ( pos.y >= h.x - pixelSize.y ) intensity += smoothstep( h.x - pixelSize.y, h.y, pos.y );\n" +
  "    if ( pos.y >= h.z - pixelSize.y ) intensity += smoothstep( h.z - pixelSize.y, h.y, pos.y );\n" +
  "  } \n" +
  "  else \n" +
  "  { \n" +
  "    if ( pos.y <= h.x + pixelSize.y ) intensity += 1.0 - smoothstep( h.y, h.x + pixelSize.y, pos.y );\n" +
  "    if ( pos.y <= h.z + pixelSize.y ) intensity += 1.0 - smoothstep( h.y, h.z + pixelSize.y, pos.y );\n" +
  "  } \n" +
  "  return intensity; \n" +
  "} \n" +

  "vec4 AddFunc" + postfix.toString() + "( in vec2 vpSize, in vec2 pos, in vec2 scale, in vec2 offs, in vec4 baseCol, in vec4 addCol ) \n" +
  "{ \n" +
  "  float func_intensity = IntensityFunc" + postfix.toString() + "( vpSize, pos, scale, offs ); \n" +
  "  return MixColor( baseCol, addCol, func_intensity ); \n" +
  "} \n";

  return codeStr;
};

// plot shader
glsl.plot_shader = {}

glsl.plot_shader.var_decl =
"uniform vec2 u_scale; \n" +
"uniform vec2 u_offset; \n" +
"uniform vec2 u_grid; \n" +
"const float c_pi = 3.14159265359; \n" +
"const vec4 frame_color  = vec4( 0.5, 0.5, 0.5, 1.0 ); \n" +
"const vec4 grid_color   = vec4( 0.5, 0.5, 0.5, 1.0 ); \n" +
"const vec4 axis_color   = vec4( 0.0, 0.0, 0.0, 1.0 ); \n";

glsl.plot_shader.InputParamDecl = function ( postfix ) {
  var code = "uniform float p" + postfix.toString() + "; \n";
  return code;
}

glsl.plot_shader.FunctionUniform = function ( postfix ) {
  var code = "  uniform vec4 f" + postfix.toString() + "_color; \n";
  return code;
}

glsl.plot_shader.start =
"void main() \n" +
"{ \n" +
"  vec4  pixel_info = PixelSizeInfo( u_view_port_size ); \n" +
"  float ratio      = pixel_info.z; \n" +
"  vec2  originRel  = -u_offset / u_scale; \n" +
"  vec4  fragCol    = vec4( 1.0 ); \n" +
"  fragCol = AddGrid( u_view_port_size, vTexCoord, u_scale, u_offset, u_grid, pixel_info, fragCol, grid_color ); \n" +
"  fragCol = AddLine( vTexCoord, vec2( 0.0, 0.0 ), vec2( 1.0, 0.0 ), pixel_info, fragCol, frame_color ); \n" +
"  fragCol = AddLine( vTexCoord, vec2( 0.0, 1.0 ), vec2( 1.0, 1.0 ), pixel_info, fragCol, frame_color ); \n" +
"  fragCol = AddLine( vTexCoord, vec2( 0.0, 0.0 ), vec2( 0.0, 1.0 ), pixel_info, fragCol, frame_color ); \n" +
"  fragCol = AddLine( vTexCoord, vec2( 1.0, 0.0 ), vec2( 1.0, 1.0 ), pixel_info, fragCol, frame_color ); \n" +
"  fragCol = AddLine( vTexCoord, vec2( originRel.x, 0.0 ), vec2( originRel.x, 1.0 ), pixel_info, fragCol, axis_color ); \n" +
"  fragCol = AddLine( vTexCoord, vec2( 0.0, originRel.y ), vec2( 1.0, originRel.y ), pixel_info, fragCol, axis_color ); \n";

glsl.plot_shader.PlotFunctionCall = function ( postfix ) {
  var code = "  fragCol = AddFunc" + postfix.toString() + "( u_view_port_size, vTexCoord, u_scale, u_offset, fragCol, f" + postfix.toString() + "_color ); \n";
  return code;
}

glsl.plot_shader.end =
"  gl_FragColor = vec4( fragCol.rgb, 1.0 ); \n" +
"} \n";

glsl.plot_shader.UpdatePlotShader = function( prog, silent, func_expr, noOfInputParam ) {

  var multi_func = Array.isArray && Array.isArray( func_expr ) ? true : false;
  var noOfFunc = multi_func ? func_expr.length : 1;

  var plot_shader_frag = {};
  plot_shader_frag.type = "frag";
  plot_shader_frag.code = [];
  plot_shader_frag.code.push( glsl.fragment_shader.std_head, glsl.fragment_shader.common_uniforms, glsl.fragment_shader.draw2D_functions );
  if ( noOfInputParam ) {
    for ( var i_param = 0; i_param < noOfInputParam; ++ i_param )
      plot_shader_frag.code.push( this.InputParamDecl( i_param.toString() ) );
  }
  if ( multi_func ) {
    for ( var i_func = 0; i_func < noOfFunc; ++ i_func )
      plot_shader_frag.code.push( glsl.fragment_shader.PlotFunction( i_func.toString(), func_expr[i_func].toString() ) );
  } else {
    plot_shader_frag.code.push( glsl.fragment_shader.PlotFunction( "0", func_expr.toString() ) );
  }
  for ( var i_func = 0; i_func < noOfFunc; ++ i_func )
    plot_shader_frag.code.push( this.FunctionUniform( i_func.toString() ) );
  plot_shader_frag.code.push( this.var_decl, this.start );
  for ( var i_func = 0; i_func < noOfFunc; ++ i_func )
    plot_shader_frag.code.push( this.PlotFunctionCall( i_func.toString() ) );
  plot_shader_frag.code.push( this.end );

  var newPlotShaderProgram = gl_util.use_new_vs_fs( glsl.vertex_shader.screenspace, plot_shader_frag, silent );
  if ( newPlotShaderProgram ) {
    gl_util.deleteProgram( plotShaderProgram );
    prog = newPlotShaderProgram;
    glsl.vertex_shader.link_uniforms( prog );
    prog.u_scale  = gl.getUniformLocation( prog, "u_scale" );
    prog.u_offset = gl.getUniformLocation( prog, "u_offset" );
    prog.u_grid   = gl.getUniformLocation( prog, "u_grid" );
    if ( noOfInputParam ) {
      prog.u_pX = []; 
      for ( var i_param = 0; i_param < noOfInputParam; ++ i_param )
        prog.u_pX.push( gl.getUniformLocation( prog, "p" + i_param.toString() ) );
    }
    prog.u_fX_color = []; 
    for ( var i_func = 0; i_func < noOfFunc; ++ i_func )
      prog.u_fX_color.push( gl.getUniformLocation( prog, "f" + i_func.toString() + "_color" ) );
    return prog;
  }

  return null;
};

glsl.plot_shader.DrawPlot = function( gl, prog, param_vals, col_vals, scale_val, offset_val, grid_val, download_request ) {

  if ( prog.u_pX ) {
    for ( var i_param = 0; i_param < prog.u_pX.length; ++ i_param ) {
      var param_val = (param_vals && param_vals.length > i_param) ? param_vals[i_param] : 0.0;
      gl.uniform1f( prog.u_pX[i_param], param_val );
    }
  }
  if ( prog.u_fX_color ) {
    for ( var i_func = 0; i_func < prog.u_fX_color.length; ++ i_func ) {
      var col_val = (col_vals && col_vals.length > i_func) ? col_vals[i_func] : "#000000";
      var col = convertUtil.hexToRgb( col_val );
      gl.uniform4fv( prog.u_fX_color[i_func], [col.r/255.0, col.g/255.0, col.b/255.0, 1.0] );
    }
  }
  gl.uniform2fv( prog.u_scale,  scale_val );
  gl.uniform2fv( prog.u_offset, offset_val );
  gl.uniform2fv( prog.u_grid,   grid_val );

  if ( download_request ) {
    if ( prog.plotFB == undefined ) {
      prog.plotFB = gl_util.createTextureFB( false, gl.viewportWidth, gl.viewportHeight );
    }
    gl.bindFramebuffer( gl.FRAMEBUFFER, prog.plotFB );
    gl.activeTexture( gl.TEXTURE0 );
    gl.bindTexture( gl.TEXTURE_2D, prog.plotFB.color0_texture );
    gl.framebufferTexture2D( gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, prog.plotFB.color0_texture, 0 );
  }

  draw2D.screenspace( gl, prog, true );

  if ( download_request ) {
    
    var w = prog.plotFB.width;
    var h = prog.plotFB.height;
    var readPixels = new Uint8Array( w * h * 4);
    gl.readPixels( 0, 0, w, h, gl.RGBA, gl.UNSIGNED_BYTE, readPixels );
    
    gl.framebufferTexture2D( gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, null, 0 );
    gl.bindFramebuffer( gl.FRAMEBUFFER, null );
    gl.bindTexture( gl.TEXTURE_2D, null );

    // flip Y
    var pixels = new Uint8Array( w * h * 4);
    for ( var i_x = 0; i_x < 4 * w; ++ i_x ) {
      for ( var i_y = 0; i_y < h; ++ i_y ) {
        var i_src = i_y * w * 4 + i_x;
        var i_dest = (h - (i_y+1)) * w * 4 + i_x;
        pixels[i_dest] = readPixels[i_src];
      }
    }

    return pixels;
  }
  return null;
};

glsl.plot_shader.AutoGridDist = function( range ) {
  var cval = 1.0;
  if ( range <= 1.0 ) {
    while ( range * cval <= 1.0 ) {
      cval *= 10.0;
    }
  } else {
    var cval = 1.0;
    while ( range * cval > 10.0 ) {
      cval /= 10.0;
    }
  }
  var dist = 1.0 / cval;
  if ( range * cval <= 1.0 ) {
    dist /= 10.0;
  } else if ( range * cval <= 2.0 ) {
    dist /= 5.0; 
  } else if ( range * cval <= 5.0 ) {
    dist /= 2.0;
  }
  return dist;
};

// steep parallax
glsl.steep_parallax =
"uniform int u_mask_mode; \n" +
"uniform int u_hm_to_nm_mode; \n" +
"uniform vec2 u_map_tex_size; \n" +
"uniform sampler2D u_maskSampler; \n" +
"uniform sampler2D u_heightSampler; \n" +
"uniform sampler2D u_normalSampler; \n" +

"float Mask( vec2 texCoords ) \n" +
"{ \n" +
"  return texture2D( u_maskSampler, texCoords ).x; \n" +
"} \n" +

"float CalculateHeight( in vec2 texCoords ) \n" +
"{ \n" +
"  float height = texture2D( u_heightSampler, texCoords ).x; \n" +
"  if ( u_mask_mode == 1 )" +
"    height *= Mask( texCoords );" +
"  return height; \n" +
"} \n" +

"vec3 CalculateNormal( vec2 texCoords ) \n" +
"{ \n" +
"  vec3 texNV = vec3( 0.0, 0.0, 1.0 ); \n" +
"  if ( u_hm_to_nm_mode != 1 ) \n" +
"  { \n" +
"    texNV = texture2D( u_normalSampler, texCoords ).xyz; \n" +
"    return normalize( vec3( texNV.xy * 2.0 - 1.0, texNV.z ) ); \n" +
"  } \n" +
"\n" +
"  // [6][7][8]    n.x = scale * -(h[2]-h[0]+2*(h[5]-h[3])+h[8]-h[6]) \n" +
"  // [3][4][5]    n.y = scale *  (h[6]-h[0]+2*(h[7]-h[1])+h[8]-h[2]) \n" +
"  // [0][1][2]    n.z = 1.0 \n" +
"  //              n = normalize(n) \n" +
"  vec2 p_size = 1.0 / u_map_tex_size; \n" +
"  float h6 = texture2D( u_heightSampler, texCoords + vec2( -p_size.x, -p_size.y ) ).x; \n" +
"  float h7 = texture2D( u_heightSampler, texCoords + vec2( 0.0, -p_size.y ) ).x; \n" +
"  float h8 = texture2D( u_heightSampler, texCoords + vec2( p_size.x, -p_size.y ) ).x; \n" +
"  float h3 = texture2D( u_heightSampler, texCoords + vec2( -p_size.x, 0.0 ) ).x; \n" +
"  float h5 = texture2D( u_heightSampler, texCoords + vec2( +p_size.x, 0.0 ) ).x; \n" +
"  float h0 = texture2D( u_heightSampler, texCoords + vec2( -p_size.x, p_size.y ) ).x; \n" +
"  float h1 = texture2D( u_heightSampler, texCoords + vec2( 0.0, p_size.y ) ).x; \n" +
"  float h2 = texture2D( u_heightSampler, texCoords + vec2( p_size.x, p_size.y ) ).x; \n" +
"  float scale = 8.0; \n" +
"  texNV.x = scale * -(h2-h0+2.0*(h5-h3)+h8-h6); \n" +
"  texNV.y = scale *  (h6-h0+2.0*(h7-h1)+h8-h2); \n" +
"  texNV = normalize( texNV ); \n" +
"  return texNV; \n" +
"} \n" +

"uniform float u_bump_scale; \n" +
"uniform float u_quality; \n" +

"vec2 SteepParallax( vec3 texDir3D, vec2 texCoord ) \n" +
"{ \n" +
"  float mapHeight; \n" +
"  float maxBumpHeight = u_bump_scale; \n" +
"  if ( maxBumpHeight > 0.01 && maxBumpHeight < 0.99 && texDir3D.z < 0.9994 ) \n" +
"  { \n" +
"    // calculate number of test steps dependet on angle of view \n" +
"    float numSteps = clamp( mix( 5.0, u_quality * clamp( 1.0 + 30.0 * maxBumpHeight, 1.0, 4.0 ), ( 1.0 - abs( texDir3D.z ) ) ), 5.0, 50.0 ); \n" +
"    const int numBinarySteps = 5; \n" +

"    // step direction on texture \n" +
"    vec2 texDir = -texDir3D.xy / texDir3D.z; \n" +

"    // init texture start coordinates \n" +
"    texCoord.xy += texDir * maxBumpHeight / 2.0; \n" +
"    vec2 texStep = texDir * maxBumpHeight; \n" +

"    // hit height step \n" +
"    float bumpHeightStep = 1.0 / numSteps; \n" +

"    // while actual bump map height is greater or equal than actual hit height \n" +
"    mapHeight = 1.0; \n" +
"    float bestBumpHeight = 1.0; \n" +
"    for ( int step = 0; step < 50; step ++ ) \n" +
"    { \n" +
"      // calcualte height on new texel \n" +
"      mapHeight = CalculateHeight( texCoord.xy - bestBumpHeight * texStep.xy ); \n" +
"      if ( mapHeight >= bestBumpHeight ) \n" +
"        break; \n" +
"      // decrease hit height and step forward on texture \n" +
"      bestBumpHeight -= bumpHeightStep; \n" +
"      if ( bestBumpHeight < 0.0 ) \n" +
"        break; \n" +
"    } \n" +
"    bestBumpHeight += bumpHeightStep; \n" +

"    // binary steps \n" +
"    for ( int i = 0; i < numBinarySteps; ++ i ) \n" +
"    { \n" +
"      bumpHeightStep *= 0.5; \n" +
"      bestBumpHeight -= bumpHeightStep; \n" +
"      // calcualte height on new texel \n" +
"      mapHeight = CalculateHeight( texCoord.xy - bestBumpHeight * texStep.xy ); \n" +
"      bestBumpHeight += ( bestBumpHeight < mapHeight ) ? bumpHeightStep : 0.0; \n" +
"    } \n" +

"    // parallax occlusion \n" +
"    bestBumpHeight -= bumpHeightStep * clamp( ( bestBumpHeight - mapHeight ) / bumpHeightStep, 0.0, 1.0 ); \n" +

"    // calculate parallax texture coordinates \n" +
"    texCoord -= bestBumpHeight * texStep; \n" +
"  } \n" +
"  else  \n" +
"    mapHeight = CalculateHeight( texCoord.xy ); \n" +
  
"  return texCoord.xy; \n" +
"} \n";


// simple phong light model 
glsl.ADS_phong =
"const vec4 u_ambientEnvironment = vec4( 0.4, 0.4, 0.4, 1.0 ); \n" +

"const vec4 u_positionLight0 = vec4( 1.0, 1.0, 3.0, 0.0 ); \n" +
"const vec4 u_ambientLight0 = vec4( 0.0, 0.0, 0.0, 1.0 ); \n" +
"const vec4 u_diffuseLight0 = vec4( 0.7, 0.7, 0.7, 1.0 ); \n" +
"const vec4 u_specularLight0 = vec4( 0.8, 0.8, 0.8, 1.0 ); \n" +

"vec3 ADS( vec3 esPt, vec3 esPtNV, vec4 ambientCol, vec4 diffuseCol, vec4 specularCol, float shininess ) \n" +
"{ \n" +
"  vec3 lightColor = vec3( 0.0 ); \n" +
"  lightColor = ambientCol.rgb * u_ambientEnvironment.rgb; \n" +
  
"  vec3 esVLight = normalize( u_positionLight0.rgb ); \n" +
"  float VNdotVL = dot( esPtNV, esVLight ); \n" +
  
"  lightColor += ambientCol.rgb * u_ambientLight0.rgb; \n" +
"  if ( VNdotVL > 0.0 ) \n" +
"  { \n" +
"    float fDiffuse = VNdotVL; \n" +
"    lightColor += diffuseCol.rgb * u_diffuseLight0.rgb * fDiffuse; \n" +

"    vec3 esVEye = normalize( -esPt ); \n" +
"    vec3 vReflect = normalize( esVEye + esVLight ); \n" +
"    float fSpecular = pow( max( dot( esPtNV, vReflect ), 0.0 ), 1.0 * shininess ); \n" +
"    //vec3 vReflect = normalize( -reflect( esVLight, esPtNV ) ); \n" +
"    //float fSpecular = pow( max( dot( vReflect, esVEye ), 0.0 ), 0.3 * shininess ); \n" +
"    lightColor += specularCol.rgb * u_specularLight0.rgb * fSpecular; \n" +
"  } \n" +
  
"  return lightColor; \n" +
"} \n";


// enhanced light model
glsl.ADS_enhanced =
"uniform int   u_rim_lighting; \n" +
"uniform float u_material_specular; \n" +
"uniform float u_material_specular_tint; \n" +
"uniform float u_material_shininess; \n" +
"uniform float u_material_roughness; \n" +
"uniform float u_material_fresnel_0; \n" +
"uniform float u_material_metallic; \n" +
"uniform float u_material_sheen; \n" +
"uniform float u_material_sheen_tint; \n" +
"uniform float u_material_anisotropic; \n" +
"uniform float u_material_subsurface; \n" +
"uniform float u_material_clearcoat; \n" +
"uniform float u_material_clearcoat_gloss; \n" +

"const vec4 u_specularMaterial = vec4( 1.0, 1.0, 1.0, 1.0 ); \n" +

"const vec4 u_ambientEnvironment = vec4( 0.4, 0.4, 0.4, 1.0 ); \n" +

"uniform vec4 u_positionLight0; \n" +
"const vec4 u_ambientLight0 = vec4( 0.0, 0.0, 0.0, 1.0 ); \n" + 
"const vec4 u_diffuseLight0 = vec4( 1.0, 1.0, 1.0, 1.0 ); \n" +
"const vec4 u_specularLight0 = vec4( 1.0, 1.0, 1.0, 1.0 ); \n" + 

"const float PI = 3.14159265358979323846; \n" +

"float Fresnel_Schlick( float theta ) \n" +
"{ \n" +
"    float m = clamp( 1.0 - theta, 0.0, 1.0 ); \n" +
"    float m2 = m * m; \n" +
"    return m2 * m2 * m; // pow( m, 5.0 ) \n" +
"} \n" +

"float GTR_Berrey( float NdotH, float al ) // GTR gamma=1 \n" +
"{ \n" +
"  if ( al >= 1.0 ) return 1.0 / PI; \n" +
"  float al2 = al * al; \n" +
"  float t = 1.0 + ( al2 - 1.0 ) * NdotH * NdotH; \n" +
"  return ( al2 - 1.0 ) / ( PI * log( al2 ) * t ); \n" +
"} \n" +

"float GTR_TrowbridgeReitz( float NdotH, float al ) \n" +
"{ \n" +
"  float al2 = al * al; \n" +
"  float t = 1.0 + ( al2 - 1.0 ) * NdotH * NdotH; \n" +
"  return al2 / ( PI * t * t ); \n" +
"} \n" +

"float GTR_2_anisotropic( float NdotH, float HdotX, float HdotY, float alx, float aly ) \n" +
"{ \n" +
"  float tx = HdotX / alx; \n" +
"  float ty = HdotY / aly; \n" +
"  float t = tx * tx + ty * ty + NdotH * NdotH; \n" +
"  return 1.0 / ( PI * alx * aly * t * t ); \n" +
"} \n" +

"float G_Schlick( float cosTheta, float m ) \n" +
"{ \n" +
"  float k = m * 0.7979; // 0.7979 ~ sqrt( 2.0 / 3.14159 ); \n" +
"  return cosTheta / ( cosTheta * ( 1.0 - k ) + k ); \n" +
"} \n" +

"float G_GGX( float cosTheta, float al ) \n" +
"{ \n" +
"  float al2 = al * al; \n" +
"  return 1.0 / ( cosTheta + sqrt( al2 + (1.0 - al2) * cosTheta * cosTheta ) ); \n" +
"} \n" +

"float G_GGX_anisotropic( float NdotV, float VdotX, float VdotY, float alx, float aly ) \n" +
"{ \n" +
"  float tx = VdotX * alx; \n" +
"  float ty = VdotY / aly; \n" +
"  return 1.0 / ( NdotV + sqrt( tx * tx + ty * ty + NdotV * NdotV ) ); \n" +
"} \n" +

"vec3 Mon_to_Lin( vec3 col ) \n" +
"{ \n" +
"  return vec3( pow( col.r, 2.2 ), pow( col.g, 2.2 ), pow( col.b, 2.2 ) ); \n" +
"} \n" +

"vec3 ADS( vec3 esPt, vec3 esPtNV, vec3 tangent, vec4 ambientCol, vec4 diffuseCol, vec4 specularCol ) \n" +
"{ \n" +
"  vec3 lightColor = vec3( 0.0 ); \n" +
"  lightColor = ambientCol.rgb * u_ambientEnvironment.rgb; \n" +
  
"  vec3 esVLight       = normalize( u_positionLight0.rgb ); \n" +
"  vec3 esVEye         = normalize( -esPt ); \n" +
"  vec3 vec_X          = normalize( tangent ); \n" +
"  vec3 vec_Y          = normalize( cross( esPtNV, vec_X ) ); \n" +
"  vec3 halfVector     = normalize( esVEye + esVLight ); \n" +
"  vec3 reflVector     = normalize( reflect( -esVLight, esPtNV ) ); \n" +
"  float VdotL         = dot( esVEye, esVLight ); \n" +
"  float VdotR         = dot( esVEye, reflVector ); \n" +
"  float HdotL         = dot( halfVector, esVLight ); \n" +
"  //float HdotV         = dot( halfVector, esVEye ); // == HdotL \n" +
"  float LdotX         = dot( esVLight, vec_X ); \n" +
"  float LdotY         = dot( esVLight, vec_Y ); \n" +
"  float VdotX         = dot( esVEye, vec_X ); \n" +
"  float VdotY         = dot( esVEye, vec_Y ); \n" +
"  float HdotX         = dot( halfVector, vec_X ); \n" +
"  float HdotY         = dot( halfVector, vec_Y ); \n" +
"  float NdotL         = dot( esPtNV, esVLight ); \n" +
"  float NdotV         = dot( esPtNV, esVEye ); \n" +
"  float NdotH         = dot( esPtNV, halfVector ); \n" +
  
"  //float fresnel0        = u_material_fresnel_0; \n" +
"  float roughness       = u_material_roughness; \n" +
"  float m2              = roughness * roughness; \n" +
"  float FD90            = 0.5 + 2.0 * HdotL * HdotL * roughness; \n" +
"  float FSS90           = HdotL * HdotL * roughness; \n" +
"  float specular        = u_material_specular; \n" +
"  float specular_tint   = u_material_specular_tint; \n" +
"  float metallic        = u_material_metallic; \n" +
"  float sheen           = u_material_sheen; \n" +
"  float sheen_tint      = u_material_sheen_tint; \n" +
"  float anisotropic     = u_material_anisotropic; \n" +
"  float subsurface      = u_material_subsurface; \n" +
"  float clearcoat       = u_material_clearcoat; \n" +
"  float clearcoat_gloss = u_material_clearcoat_gloss; \n" +

"  float err_d = step( 0.0, NdotL ) + step( NdotL, 0.0 ) * pow( 1.0 + NdotL, 5.0 ); \n" +
"  //float err_s = step( 0.0, NdotL ) * step( 0.0, NdotV ); \n" +
"  float err_s = err_d; \n" +
"  NdotL = abs( NdotL ); \n" +
"  NdotV = clamp( NdotV, 0.0, 1.0 ); \n" +
  
"  float F_NdotL = Fresnel_Schlick( NdotL ); \n" +
"  float F_NdotV = Fresnel_Schlick( NdotV ); \n" +
"  float F_HdotL = Fresnel_Schlick( HdotL ); \n" +
  
"  //vec3 diffus_lin = Mon_to_Lin( diffuseCol.rgb ); \n" +
"  vec3 diffus_lin = diffuseCol.rgb; \n" +
"  //vec3 specular_lin = Mon_to_Lin( specularCol.rgb ); \n" +
"  vec3 specular_lin = specularCol.rgb; \n" +

"  // luminance approximation \n" +
"  float spec_lum = specular_lin.r * 0.3 + specular_lin.g * 0.6 + specular_lin.b * 0.1; \n" + 
"  vec3 col_spec_tint = spec_lum > 0.0 ? specular_lin / spec_lum : vec3( 1.0 ); // normalize luminance to isolate hue+sat \n" +
    
"  vec3 col_spec = mix( vec3(1.0), col_spec_tint, specular_tint ); \n" +  
"  vec3 col_spec_0 = mix( specular * 0.08 * col_spec, specular_lin, metallic ); \n" +
"  vec3 col_sheen = mix( vec3(1.0), col_spec_tint, sheen_tint ); \n" +

"  // diffuse energy ratio Burley at Disney \n" +
"  float Fd = ( 1.0 + ( 1.0 - FD90 ) * F_NdotL ) * ( 1.0 + ( 1.0 - FD90 ) * F_NdotV ); \n" +

"  // Based on Hanrahan-Krueger BRDF approximation of isotropic BSSRDF \n" +
"  // 1.25 scale is used to (roughly) preserve albedo \n" +
"  // FSS90 used to 'flatten' retroreflection based on roughness \n" +
"  float Fss = ( 1.0 + ( 1.0 - FSS90 ) * F_NdotL ) * ( 1.0 + ( 1.0 - FSS90 ) * F_NdotV ); \n" +
"  float ss = 1.25 * (Fss * (1.0 / (NdotL + NdotV) - 0.5) + 0.5); \n" +

"  // specular distribution \n" +
"  float aspect = sqrt( 1.0 - anisotropic * 0.9 ); \n" +
"  float al_x = max( 0.00001, m2 / aspect ); \n" +
"  float al_y = max( 0.00001, m2 * aspect ); \n" +
"  float Ds; \n" +
"  if ( anisotropic <= 0.01 ) \n" +
"    Ds = GTR_TrowbridgeReitz( NdotH, m2 ); \n" + 
"  else \n" +
"    Ds = GTR_2_anisotropic( NdotH, HdotX, HdotY, al_x, al_y ); \n" +
"  // specular fresnell \n" +
"  vec3 Fs = col_spec_0 + ( 1.0 - col_spec_0 ) * F_HdotL; \n" +
"  // specular geometry term \n" +
"  float m_ggx = (0.5 + roughness/2.0); \n" +
"  float m2_ggx = m_ggx * m_ggx; \n" +
"  float Gs; \n" +
"  if ( anisotropic <= 0.01 ) \n" +
"    Gs = G_GGX( NdotL, m2_ggx ) * G_GGX( NdotV, m2_ggx ); \n" +
"  else \n" +
"  { \n" +
"    float al_x_ggx = max( 0.00001, m2_ggx / aspect ); \n" +
"    float al_y_ggx = max( 0.00001, m2_ggx * aspect ); \n" +
"    Gs = G_GGX_anisotropic( NdotL, LdotX, LdotY, al_x_ggx, al_y_ggx ) * G_GGX_anisotropic( NdotV, VdotX, VdotY, al_x_ggx, al_y_ggx ); \n" +
"  } \n" +

"  // sheen \n" +
"  vec3 Fsheen = F_HdotL * sheen * col_sheen; \n" +

"  // clearcoat (ior = 1.5 -> F0 = 0.04) \n" +
"  float Dr = GTR_Berrey( NdotH, mix( 0.1, 0.001, clearcoat_gloss ) ); \n" +
"  float Fr = mix( 0.04, 1.0, F_HdotL ); \n" +
"  float Gr = G_Schlick( NdotL, 0.25 ) * G_Schlick( NdotV, 0.25 ); \n" +

"  // diffuse material color \n" +
"  vec3 diffuse_material = ( (1.0 / PI ) * mix( Fd, ss, subsurface ) * diffus_lin + Fsheen ) * ( 1.0 - metallic); \n" +
"  // specualr material color \n" +
"  vec3 specular_material = Gs * Fs * Ds + 0.25 * clearcoat * Gr * Fr * Dr; \n" +

"  diffuse_material *= err_d; \n" +
"  specular_material *= err_s; \n" +

"  // rim lighting \n" +
"  if ( u_rim_lighting != 0 ) \n" +
"  { \n" +
"    float k_rim_v = pow( 1.0 - abs( NdotV ), 4.0 ); \n" +
"    float k_rim_lv = pow( max( 0.0, -VdotL ), 2.0 ); \n" +
"    float k_rim = k_rim_v * k_rim_lv; \n" +
"    vec3 spec_rim = mix( specular_lin, col_spec_tint, specular_tint * ( 1.0 - metallic ) ); \n" +
"    vec3 specular_rim = k_rim * mix( mix( diffus_lin, spec_rim, specular ), col_sheen, sheen ); \n" +
"    specular_material = max( specular_material, specular_rim ); \n" +
"  } \n" +
  
"  // mix light \n" +
"  lightColor += err_d * diffuse_material * u_diffuseLight0.rgb; \n" +
"  lightColor += specular_material * u_specularLight0.rgb; \n" +
  
"  return lightColor; \n" +
"} \n";


//! \class glsl
//!
//!
var draw2D = {};

//! draw screen space quad
draw2D.screenspace = function( gl, prog, clearFB, viewMat, texMat ) {

  // create screen space mesh
  if ( !draw2D.meshQuad ) {
    draw2D.meshQuad = mesh.createQuad();
    mesh.createBuffer( draw2D.meshQuad );
  }
 
  // init viewport
  gl.viewport( 0, 0, gl.viewportWidth, gl.viewportHeight );
  // clear frame buffer
  if ( clearFB && clearFB != false ) {
    gl.clear( gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT );
  }

  // define matrix uniform initializations
  var mat4_proj = mat4.identity(); 
  var mat4_view = mat4.identity(); 
  if ( viewMat )
    mat4_view = mat4_view;
  var mat3_view = mat4.toMat3( mat4_view );
  var mat4_texture = mat4.identity(); 
  if ( texMat )
    mat4_texture = texMat;  

  // init matrix uniforms
  if ( prog.u_matProjection )
    gl.uniformMatrix4fv( prog.u_matProjection, false, mat4_proj );
  if ( prog.u_matModelView )
    gl.uniformMatrix4fv( prog.u_matModelView, false, mat4_view );
  if ( prog.u_matNormal )
    gl.uniformMatrix3fv( prog.u_matNormal, false, mat3_view );  
  if ( prog.u_matTexture )
    gl.uniformMatrix4fv( prog.u_matTexture, false, mat4_texture );

  // init common uniforms
  if ( prog.u_view_port_size )
    gl.uniform2fv( prog.u_view_port_size, [ gl.viewportWidth, gl.viewportHeight ] );
  
  // draw scenn space quad
  mesh.draw( draw2D.meshQuad, prog.aVertPos, prog.aNormalVec, prog.aTexCoord );
}

//! \class convertUtil
//!
//!
var convertUtil = {};

// http://stackoverflow.com/questions/5623838/rgb-to-hex-and-hex-to-rgb
convertUtil.componentToHex = function(c) {
    var hex = c.toString(16);
    return hex.length == 1 ? "0" + hex : hex;
}
convertUtil.rgbToHex = function(r, g, b) {
    return "#" + componentToHex(r) + componentToHex(g) + componentToHex(b);
    //return "#" + ((1 << 24) + (r << 16) + (g << 8) + b).toString(16).slice(1);
}
convertUtil.hexToRgb = function(hex) {
    // Expand shorthand form (e.g. "03F") to full form (e.g. "0033FF")
    var shorthandRegex = /^#?([a-f\d])([a-f\d])([a-f\d])$/i;
    hex = hex.replace(shorthandRegex, function(m, r, g, b) {
        return r + r + g + g + b + b;
    });

    var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
    return result ? {
        r: parseInt(result[1], 16),
        g: parseInt(result[2], 16),
        b: parseInt(result[3], 16)
    } : null;
}