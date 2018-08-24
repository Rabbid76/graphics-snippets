# Transformation of the model

## Rotation matrix

Rotation matrix, wikipedia [https://en.wikipedia.org/wiki/Rotation_matrix]
> In linear algebra, a rotation matrix is a matrix that is used to perform a rotation in [Euclidean space](https://en.wikipedia.org/wiki/Euclidean_space)

See [Matrix operations](https://github.com/Rabbid76/graphics-snippets/blob/master/documentation/matrix_operations.md)


## Euler angles

Euler angles, wikipedia [https://en.wikipedia.org/wiki/Euler_angles]
> The Euler angles are three angles introduced by Leonhard Euler to describe the orientation of a rigid body with respect to a fixed coordinate system. They can also represent the orientation of a mobile frame of reference in physics or the orientation of a general basis in 3-dimensional linear algebra.

Euler's rotation theorem, wikipedia [https://en.wikipedia.org/wiki/Euler%27s_rotation_theorem]
> In geometry, Euler's rotation theorem states that, in three-dimensional space, any displacement of a rigid body such that a point on the rigid body remains fixed, is equivalent to a single rotation about some axis that runs through the fixed point. It also means that the composition of two rotations is also a rotation. Therefore the set of rotations has a group structure, known as a rotation group.


## Quaternion

Quaternion, wikipedia [https://en.wikipedia.org/wiki/Quaternion]
> In mathematics, the quaternions are a number system that extends the complex numbers. They were first described by Irish mathematician William Rowan Hamilton in 1843 and applied to mechanics in three-dimensional space. A feature of quaternions is that multiplication of two quaternions is non commutative. Hamilton defined a quaternion as the quotient of two directed lines in a three-dimensional space or equivalently as the quotient of two vectors.
>
> Quaternions are generally represented in the form:
>
> a + b ***i*** + c ***j*** + c ***k*** 


Quaternions and spatial rotation, wikipedia  [https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation]

> Unit quaternions, also known as versors, provide a convenient mathematical notation for representing orientations and rotations of objects in three dimensions. Compared to Euler angles they are simpler to compose and avoid the problem of gimbal lock. Compared to rotation matrices they are more compact, more numerically stable, and more efficient.

Unit vectors ***i***, ***j***, ***k***:

    i = (1, 0, 0)
    j = (0, 1, 0)
    k = (0, 0, 1)

Representation of a cartesian vector (x, y, z)

    v = (x, y, z) = x*i + y*j + z*k

An rotation around an arbitrary axis can be described by an an unit vector ***u*** (axis) and a rotation ***theta*** (roll) around this vector.

TODO: image

This orientation can be represented by a quaternion:

    q = cos(theta/2) + (ux*i + uy*j + uz*k) * sin(theta/2)

A rotation matrix which does a transformation which is described by the quaternion can be achieved like this:

    q = qr + qi*i + qj*j + qk*k

    p' = q*p*q' = R*p
 
        [ 1 - 2*s*(qj*qj + qk*qk),  2*s*(qi*qj - qk*qr),     2*s*(qi*qk + qj*qr)     ]
    R = [ 2*s*(qi*qj + qk*qr),      1 - 2*s*(qi*qi + qk*qk), 2*s*(qj*qk - qi*qr)     ]
        [ 2*s*(qi*qk - qj*qr),      2*s*(qj*qk + qi*qr),     1 - 2*s*(qi*qi + qj*qj) ]

    s = ||q||^-2, if q is a unit quaternion, s=1.

Get the orientation axis and the rotation angle from a quaternion:

    |q|       = sqrt(qi*qi + qj*qj + qk*qk)

    (x, y, z) = (qi, qr, qk) / |q|

    theta     = 2 * atan2(|q|, qr)


The orthogonal matrix corresponding to a rotation by the unit quaternion 

    z = a + b*i + c*j + d*k, |z| = 1
  
when post-multiplying with a column vector is given by

        [ a*a + b*b - c*c - d*d,  2*b*c - 2*a*d,         2*b*d + 2*a*c        ]
    R = [ 2*b*c + 2*a*d,          a*a - b*b + c*c - d*d, 2*c*d - 2*a*b        ]
        [ 2*b*d - 2*a*c,          2*c*d + 2*a*b,         a*a -b*b - c*c + d*d ]


### Rotate by quaternion

Express [`glm::rotate`](https://glm.g-truc.net/0.9.8/api/a00232.html#ga2020c91bf61e050882b3a5c18eada700) by an quaternion:

    glm::rotate( ang_rad, (ux, uy, uz))v
   
    (ux, uy, uz): rotation axis (unit vector)
    ang_rad:      rotation angle in radians

    c = cos(ang_rad/2)
    s = sin(ang_rad/2)

    q = (c, ux*s, uy*s, uz*s)


## Matrix calculus

Matrix calculus, wikipedia [https://en.wikipedia.org/wiki/Matrix_calculus]
> In mathematics, matrix calculus is a specialized notation for doing multivariable calculus, especially over spaces of matrices. It collects the various partial derivatives of a single function with respect to many variables, and/or of a multivariate function with respect to a single variable, into vectors and matrices that can be treated as single entities. This greatly simplifies operations such as finding the maximum or minimum of a multivariate function and solving systems of differential equations.


<br/><hr/>

<a href="https://stackexchange.com/users/7322082/rabbid76"><img src="https://stackexchange.com/users/flair/7322082.png" width="208" height="58" alt="profile for Rabbid76 on Stack Exchange, a network of free, community-driven Q&amp;A sites" title="profile for Rabbid76 on Stack Exchange, a network of free, community-driven Q&amp;A sites" /></a>