# Plot the function that is used for --axis-sensitivity

set title "Axis Sensitivity"
set output "sensitivity.svg"
set terminal svg
set xlabel "Axis Input"
set ylabel "Axis Output"

set xrange [0:1]
set yrange [0:1]
set size ratio 1

f(x,t) = (1 - (1 - x)**(2**t))**(1/(2**t))
g(x,t) = 1 - (1 - x**(2**(-t)))**(1/(2**(-t)))
s(x) = sgn(abs(x) - x)
h(x,t) = (1 - s(t)) * f(x,t) + s(t) * g(x,t)

plot h(x,4.0), h(x,2.0), h(x,1.0), h(x,0.5), h(x,0.0), h(x,-0.5), h(x,-1.0),  h(x,-2.0), h(x,-4.0)

# EOF #
