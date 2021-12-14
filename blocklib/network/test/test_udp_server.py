from newsched import blocks, gr
from newsched import network
from matplotlib import pyplot as plt


fg = gr.flowgraph('udp server')

udp_server = network.udp_source(8080,9000,0)

hd = blocks.head(1000)
snk = blocks.vector_sink_b()

fg.connect([udp_server, hd, snk])

fg.run()


plt.plot(snk.data())
plt.show()
