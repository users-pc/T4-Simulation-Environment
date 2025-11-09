//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
// TictocPacket.ned
// Txc1.ned
simple Txc1
{
    parameters:
        int address;              // eigene Adresse
    gates:
        input in @loose;        // darf unverbunden bleiben
        output out @loose;        // darf unverbunden bleiben
}

// Tictoc10Middlemen.ned
network TictocExtended
{
    parameters:
        int N = 10;               // Anzahl Middlemen
        @display("bgb=614,406");
    submodules:
        tic: Txc1 {
            parameters:
                address = 0;
                @display("p=85,216");
        }
        middleman[N]: Txc1 {
            parameters:
                address = index + 1;           // 1..10
                @display("p=257,216");
        }
        toc: Txc1 {
            parameters:
                address = parent.N + 1;               // 11
                @display("p=447,216");
        }
    connections:
        // tic -> erster Middleman
        tic.out --> {  delay = 100ms; } --> middleman[0].in;

        // Middleman-Kette
        for i=0..N-2 {
            middleman[i].out --> {  delay = 100ms; } --> middleman[i+1].in;
        }

        // letzter Middleman -> toc
        middleman[N-1].out --> {  delay = 100ms; } --> toc.in;
}
