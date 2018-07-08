path="./"
#input_file_name="poke_duplicator.*"
output_file_name="g_out.gcode"

x_size=100
y_size=100
x_offset=$((-x_size/2))
y_offset=$((-y_size/2))
dy_poke=0.5

x_cnt=0
y_cnt=0

x_pokes=2
y_pokes=2

g_init="$(cat << EOF
M103
M73 P0
G21
G90
EOF
)"

g_homing=$(cat << EOM
G162 X Y F2500
G161 Z F1100
G92 Z-5
G1 Z0.0
G161 Z F100
M132 X Y Z A B
EOM
)

g_safe_pos=$(cat << EOM
G1 X-60 Y-60 Z50 F3300.0
EOM
)

g_init_pos=$(cat << EOM
G1 X$(($x_size+$x_offset)) Y$(($y_size+$y_offset)) Z10 F3300.0 
EOM
)

poke_script=$(cat << EOM 
G91 
G1 F90
G1 Z-$dy_poke
G1 Z+$dy_poke
G1 Z-1
G1 Z+1
G90
EOM
)

g_end=$(cat << EOM
M73 P100 
G0 Z150
M18
M109 S0 T0
M104 S0 T0
M104 S0 T1
G162 X Y F2500
M18
M70 P5 (Roboy)
M72 P1
EOM
)

echo "$g_init" > "$output_file_name" 
echo "$g_homing" >> "$output_file_name" 
echo "$g_safe_pos" >> "$output_file_name" 
echo "$g_init_pos" >> "$output_file_name" 
for (( x_cnt=0; x_cnt <=x_size; x_cnt=x_cnt+x_size/x_pokes ))
do
   for (( y_cnt=0; y_cnt <= y_size; y_cnt=y_cnt+y_size/y_pokes ))
   do
      echo "G1 X$(($x_offset+$x_cnt)) Y$(($y_offset+$y_cnt)) Z10 F3300.0" >> "$output_file_name" 
   
   echo "$poke_script" >> "$output_file_name" 
   done
done
echo "$g_end" >> "$output_file_name" 
