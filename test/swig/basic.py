import os,sys
sys.path.append(os.path.abspath('.'));

import dnnc as dc
t1=dc.make_tensor(2,3)
t2=dc.make_tensor(3,2)

mul = dc.multiply(t1,t2)
#print ("multiplication : " , mul.to_string())
add = dc.add(t1,t1)
#print ("addition : " , add.to_string())

t3 = dc.make_tensor(2,3,4)
#print("old shape", t1.shape())
new_shape = dc.ivec([2,12])
t3.reshape(new_shape)
#print("new shape", t1.shape())

#t4 = dc.thresholded_relu(t1);
#print("relu", t4.to_string())

#replace first few values in tensor with new values.
data = dc.fvec([1.0, 2.0, 3.0, 4.0])
t3.load(data)
#print(t3.to_string())

arr = dc.array([1, 2])
#print(arr)
arr2D = dc.array([[1, 2], [10, 20]])
#print(arr2D)
arrRand = dc.random(2, 3);
#print(arrRand)
empty = dc.empty(3, 2);
#print(empty)
zeros = dc.zeros(2, 2);
#print(zeros);
ones = dc.ones(2, 2);
#print(ones)
ranges = dc.arange(15, 3, 2)
#print(ranges)


def test_multiply(a,b):
    c = dc.multiply(a, b)
    #print(c)


#3D MatMul Test1
a = dc.make_tensor(2, 2, 2)
b = dc.make_tensor(2, 2, 2)
adata = dc.fvec([1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0])
bdata = dc.fvec([8.0,7.0,6.0,5.0,4.0,3.0,2.0,1.0])
a.load(adata)
b.load(bdata)
test_multiply(a,b)

#3D MatMul Test2
a = dc.make_tensor(2, 3, 2)
b = dc.make_tensor(3, 2, 2)
adata = dc.fvec([1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0,10.0,11.0,12.0])
bdata = dc.fvec([12.0,11.0,10.0,9.0,8.0,7.0,6.0,5.0,4.0,3.0,2.0,1.0])
a.load(adata)
b.load(bdata)
test_multiply(a,b)
